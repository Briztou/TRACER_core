/*
  arm_v7m.c Part of TRACER

  Copyright (c) 2018 Raphaël Outhier

  TRACER is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  TRACER is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  aint32_t with TRACER.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
 * This source file is the only file to include in the executable to make the kernel work properly.
 * 	All other components (FPU, MPU, etc...) included in modules;
 */


//------------------------------------------------ Make parameters -----------------------------------------------

/*
 * Makefile must provide :
 * 	- NB_INTERRUPTS : the number of interrupts supported by the chip.
 */

#if !defined(NB_INTERRUPTS)

#error "The number of interrupts was not provided. Check your makefile;"

#define NB_INTERRUPTS 256

#endif


//------------------------------------------------ Includes -----------------------------------------------

#include <stdint.h>
#include <kernel/krnl.h>
#include <kernel/async/interrupt.h>
#include <kernel/panic.h>
#include <kernel/proc/proc.h>
#include <kernel/async/fault.h>


//------------------------------------------------ Arch and proc globals -----------------------------------------------

//The arm v7m has only one core and one thread;
const uint8_t proc_nb_threads = 1;


//-------------------------------------------------- Stack management  -------------------------------------------------


/*
 * Notice :
 *
 *
 *  arm v7m minimal stack frame :
 *
 *      ----------- <- pre irq / stack frame header end;
 *      XPSR
 *      PC
 *      LR
 *      R12
 *      R3
 *      R2
 *      R1
 *      R0--------- <- post irq
 */

/*
 * proc_init_stack : this function initialises the unstacking environment, so that the given function will
 *  be executed at context switch time;
 *
 *  It starts by caching the process_t stack pointer, and stacks the process functions pointers, and the PSR.
 *
 *  Then, it saves the process index in R12 (next word);
 *
 *  Finally, it leaves space for empty stack frame and saves the stack pointer;
 */

void proc_create_stack_context(struct proc_stack *stack, void (*function)(), void (*exit_loop)(), void *arg) {
	
	//Cache the current stack pointer;
	uint32_t *sp4 = stack->sp;
	
	//Store the PSR. Contains the execution mode; //TODO DOC
	*(sp4 - 1) = 0x01000000;
	
	//Store the function in PC cache
	*(sp4 - 2) = (uint32_t) (function);
	
	//Store the return function in LR;
	*(sp4 - 3) = (uint32_t) (exit_loop);
	
	//Store the arg in R12 cache;
	*(sp4 - 4) = (uint32_t) (arg);
	
	//Update the stack pointer; Hits the future R0 reload address;
	sp4 -= 8;
	
	//Return the stack pointer;
	stack->sp = sp4;
	
}


/*
 * proc_get_init_arg : this function will return the value of r12. If called by the process init function,
 *  it will be equal to the previous function's arg;
 */

void *proc_get_init_arg() {
	uint32_t arg = 0;
	__asm__ __volatile__("mov %0, r12": "=r" (arg):);
	return (void *) arg;
}


//Determine the closest inferior address, that would respect alignment requirements;
void *proc_stack_align(void *stack_reset) {
	
	//@core_stack_alignment is a power of 2, decrement and we obtain the mask of bits to reset;
	return (void *) ((((size_t) stack_reset) & ~((size_t) 7)));
	
}


//---------------------------------------------------- Threads setup ---------------------------------------------------

/**
 * core_enter_thread_mode : this function initialises threads in a safe state. It never returns.
 * 	The preemption environment must have been initialised before;
 *
 * 	- initialises all threads in their lowest privilege state;
 * 	- updates exception stacks;
 * 	- triggers the preemption;
 * 	- make all threads run an infinite loop;
 *
 * @param exception_stacks : stacks that must be used in case of interrupt;
 * @param preemption_call : the function to execute in the executing thread when all other threads are idle;
 */

void proc_enter_thread_mode(struct proc_stack **exception_stacks) {
	
	//Disable all interrupts;
	ic_disable_interrupts();
	
	//A static var will insure that the function is called at most once;
	static bool entered = false;
	
	//If the flag has already been set, the function is called for the second time.
	if (entered) {
		
		//Error, must not happen;
		kernel_panic("proc_enter_thread_mode : called a second time;");
		
	}
	
	//Mark the function called;
	entered = true;
	
	//If we are in handler mode, ignore the request;
	if (ic_in_handler_mode()) {//TODO
		
		//Error, this function can't be executed in handler mode;
		kernel_panic("proc_enter_thread_mode : in handler mode;");
		
		//Never reached;
		return;
		
	}
	
	static volatile void *volatile msp;
	static volatile void *volatile psp;
	
	//Save the exception stack value in msp;
	msp = exception_stacks[0]->sp;
	
	//Save the current main stack pointer in psp's cache;
	__asm__ __volatile__ ("mrs %0, msp" : "=r" (psp):);
	
	//Execute an ISB;
	__asm__ __volatile__ ("DMB");
	
	//Update the main stack pointer so that exceptions occur in the exception stack;
	__asm__ __volatile__ (\
            "msr msp, %0\n\t"::"r" (msp):"memory"
	);
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	
	//Update the process stack pointer;
	__asm__ __volatile__ (\
            "msr psp, %0\n\t"::"r" (psp):"memory"
	);
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	
	//Update the control register to use PSP;//TODO UNPRIVILLEGE
	__asm__ __volatile__(\
        "mov r4, #2 \n\t"\
        "msr control, r4\n\t":: :"r4", "cc", "memory"
	);
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	
	//Execute the preemption call;
	preemption_set_pending();
	
	//Enable interrupts;
	ic_enable_interrupts();
	
	kernel_panic("NO_PREEMPTION");
	
	//Wait eternally for the preemption to be triggered;
	while (1);
	
}


//----------------------------------------------------- Preemption -----------------------------------------------------


/*
 * The context switcher :
 *
 *  - saves the current thread context;
 *  - calls the scheduler;
 *  - loads the new thread context;
 */

//TODO MAKE ANOTHER ONE FOR NO FPU SAVE;

void proc_context_switcher() {
	
	//TODO ONLY IN HANDLER MODE;
	
	//A static variable will contain the temporary psp that we transmit to the stack provider;
	static void *volatile psp;
	
	/*
	 * This function happens in an interrupt basis. Will not happen during a critical section.
	 * Ints can be safely disabled;
	 */
	
	//Disable all interrupts during context switching;
	__asm__ __volatile__("cpsid i");
	
	
	//As R0 is already saved in memory, we can use it;
	__asm__ __volatile__ (\
            "mrs r0, psp \n\t"\
            "stmdb r0!, {r4 - r11}\n\t"\
            /*"vstmdb r0!, {s16 - s31}"\*/
	);
	
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	
	//Save the current process_t's stack pointer, while the process_t hasn't been terminated;
	__asm__ __volatile__ ("mrs %0, psp" : "=r" (psp):);
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	//Enable interrupts;
	__asm__ __volatile__("cpsie i");
	
	
	//Provide the old stack and get a new one; There is only one thread, with the index 0;
	psp = kernel_switch_thread_stack(0, psp);
	
	
	//Disable all interrupts during context switching;
	__asm__ __volatile__("cpsid i");
	
	//Update the PSP;
	__asm__ __volatile__ ("msr psp, %0"::"r" (psp));
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	//Unstack the context from the new stack; R0 will be updated at interrupt prempt, we can use it;
	__asm__ __volatile__ (\
            "mrs r0, psp \n\t"\
            "ldmdb r0!, {r4 - r11} \n\t"\
            /*"vldmdb r0!, {s16 - s31} "\*/
	);
	
	//Execute an ISB;
	__asm__ __volatile__ ("ISB");
	
	//Enable interrupts;
	__asm__ __volatile__("cpsie i");
	
}


//------------------------------------------------- Faults ------------------------------------------------

static void arm_v7m_hard_fault_handler() {
	kernel_handle_fault(0);
}

static void arm_v7m_mem_fault_handler() {
	kernel_handle_fault(2);
}

static void arm_v7m_bus_fault_handler() {
	kernel_handle_fault(1);
}


static void arm_v7m_usg_fault_handler() {
	kernel_handle_fault(2);
}


//------------------------------------------------- kernel vector table ------------------------------------------------

/*
 * Some data is required to start the processor properly. Namely :
 * 	- The initial stack pointer, provided by the linker;
 * 	- The first function to execute, defined in another piece of code;
 */

extern void __entry_point();

extern uint32_t _ram_highest;



//Define an empty ISR handler
void no_isr() {};

//We need the address of the kernel vector table;
void (*kernel_vtable[NB_INTERRUPTS])(void) = {
	
	//0 : not assigned;
	&no_isr,
	
	//1 : reset; Effectively used when NVIC is relocated;
	&__entry_point,
	
	//2 : NMI. Not supported for instance;
	&no_isr,
	
	//3 : HardFault.
	&arm_v7m_hard_fault_handler,
	
	//4 : MemManage fault;
	&arm_v7m_mem_fault_handler,
	
	//5 : BusFault.
	&arm_v7m_bus_fault_handler,
	
	//6 : UsageFault;
	&arm_v7m_usg_fault_handler,
	
	//7 : Reserved;
	&no_isr,
	
	//8 : Reserved;
	&no_isr,
	
	//9 : Reserved;
	&no_isr,
	
	//10 : Reserved;
	&no_isr,
	
	//11 : SVCall. Support provided by a module;
	&no_isr,
	
	//12 : Reserved;
	&no_isr,
	
	//13 : Reserved;
	&no_isr,
	
	//14 : PendSV. Support provided by a module;
	&no_isr,
	
	//15 : SysTick. Support provided by a module;
	&no_isr,
	
	//All interrupts not handled;
	[16 ... NB_INTERRUPTS - 1] = &no_isr,

};

//-------------------------------------------------- NVIC static base --------------------------------------------------



//If the vtable must not be generated (relocated after, smaller executable) :
#ifdef NOVTABLE

/*
 * In order to start the processor properly, we define here the vector table, that is hard-copied in the firmware
 * 	as it, at the link section .vector. This section can be found in the link script, and starts at address 0;
 */

static void *vtable[NB_INTERRUPTS] __attribute__ ((section(".vectors"))) = {
	
	//0 : Initial SP Value; In ARM Architecture, the stack pointer decreases;
	&_ram_highest,
	
	//1 : Reset : call the program's entry point;
	&__entry_point,
	
	//In order to avoid writing 254 times the function name, we will use macros that will write it for us;
#define channel(i) &no_isr,

//Redirect all isrs to the empty one;
#include "nvic_channel_list.h"

#undef channel
	
	//Adding another "&no_isr" will cause a compiler warning "excess array initializer. Try it, it is funny !
	
};


//If the vtable must be generated
#else //NOVTABLE


/**
 * isr_generic_handler : in order to support handler update, all functions of the in-flash vector table will
 * 	lead to this function, that executes the appropriate function;
 *
 * @param i : the interrupt channel. 0 to 240;
 */

static void isr_generic_flash_handler(uint8_t i) {
	
	//Execute the handler;
	(*kernel_vtable[i])();
	
}


/*
 * Generate an ISR for each interrupt channel; Done using XMacro;
 */

//The handler link : a function that calls the handler link with a specific value;
#define channel(i) static void isr_##i() {isr_generic_flash_handler(i);}

//Define all isrs;
#include "nvic_channel_list.h"

#undef channel


/*
 * In order to start the processor properly, we define here the vector table, that is hard-copied in the firmware
 * 	as it, at the link section .vector. This section can be found in the link script, and starts at address 0;
 */

static void *vtable[NB_INTERRUPTS] __attribute__ ((section(".vectors"))) = {
	
	//0 : Initial SP Value; In ARM Architecture, the stack pointer decreases;
	&_ram_highest,
	
	//1 : Reset : call the program's entry point;
	&__entry_point,
	
	
	//2->255 : empty handler (240 times, 240 = 3 * 8 * 10);
	
	//In order to avoid writing 254 times the function name, we will use macros that will write it for us;
#define channel(i) &isr_##i,

//Redirect all isrs to the empty one;
#include "nvic_channel_list.h"

#undef channel
	
	//Adding another "&empty_isr" will cause a compiler warning "excess array initializer. Try it, it is funny !
	
};


#endif //NOVTABLE