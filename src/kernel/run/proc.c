/*
  proc.c Part of TRACER

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

//--------------------------------------------------- Make Parameters --------------------------------------------------

//The memory library required NB_THREADS to be provided by the makefile;
#if !defined(EXCEPTION_STACKS_SIZE)

//COmpilation fail;
#error "Error, one make parameter not provided, check your makefile"

#define EXCEPTION_STACKS_SIZE 1000

#endif

//--------------------------------------------- Includes --------------------------------------------

#include <kernel/async/interrupt.h>
#include <kernel/async/preempt.h>
#include <kernel/log.h>
#include <kernel/run/sched.h>
#include <kernel/clock/sysclock.h>
#include <kernel/mem/ram.h>
#include <util/string.h>


//--------------------------------------------- Vars --------------------------------------------


//A flag set if the current process must be terminated during teh next preemption;
bool prc_process_terminated;

//The stacks array; Will reference interrupt stacks;
static struct proc_stack exception_stack;

//If required, a module can provide a function to create a stack header before the stack context. Used for ex for FPU;
static void (*stack_header_creator)(struct proc_stack *stack);


//------------------------------------------------- Proc requirements --------------------------------------------------

//The hardware library must provide a function to create the general stack context;
extern void proc_create_stack_context(struct proc_stack *stack, void (*function)(), void (*exit_loop)(), void *arg);

//The context switcher; Should be set as the preemption handler for a preemption to occur;
extern void proc_context_switcher();


/**
 * proc_enter_thread_mode : this function initialises threads in a safe state. It never returns.
 * 	The preemption environment must have been initialised before;
 *
 * 	- initialises all threads in their lowest privilege state;
 * 	- updates exception stacks;
 * 	- calls the provided function, that should trigger the preemption;
 * 	- make all threads run an infinite loop;
 * 	- execute the preemption call;
 * 	- enable interrupts;
 * 	- loop;
 *
 * @param exception_stacks : processor stacks that must be used in case of interrupt;
 */

extern void proc_enter_thread_mode(struct proc_stack *exception_stacks);


//------------------------------------------------- Proc requirements --------------------------------------------------

//Initialise the stack context for future execution;
void proc_init_stack(struct proc_stack *stack, void (*function)(), void (*end_loop)(), void *init_arg) {
	
	//Reset the stack pointer;
	stack->sp = stack->sp_reset;
	
	//Create the stack header;
	//TODO PATCH
	//create_stack_header(stack);
	
	//Create the stack context;
	proc_create_stack_context(stack, function, end_loop, init_arg);
	
}


/**
 * init_exception_stacks : initialises the exception stacks array;
 */

void init_exception_stack() {
	
	//Allocate some memory for the thread's stack in the newly created heap;
	void *thread_stack = ram_alloc(EXCEPTION_STACKS_SIZE);
	
	//Determine the stack's highest address;
	void *stack_reset = (void *) ((uint8_t *) thread_stack + EXCEPTION_STACKS_SIZE);
	
	//Correct the stack's highest address for proper alignment;
	stack_reset = proc_stack_align(stack_reset);
	
	//Create the proc_stack initializer;
	struct proc_stack cs_init = {
		
		//The stack bound, not corrected;
		.stack_limit = thread_stack,
		
		//The stack pointer, set to its reset value;
		.sp = stack_reset,
		
		//The stack reset value, corrected by the core lib;
		.sp_reset = stack_reset,
		
	};
	
	//Initialise the exception stack;
	memcpy(&exception_stack, &cs_init, sizeof(struct proc_stack));
	
}


/**
 * kernel_enter_multitasked_mode : setups the kernel for multi-tasked mode.
 */

void proc_start_execution() {
	
	//Disable all interrupts;
	exceptions_disable();
	
	//Initialise the exception stacks array;
	init_exception_stack();
	
	//Initialise the preemption;
	preemption_init(&proc_context_switcher, KERNEL_PREMPTION_PRIORITY);
	
	//Log;
	kernel_log_("preemption initialised");
	
	
	/*
	 * Syscalls;
	 */
	
	/*
	//Set the syscall handler; Same prio as preemption;
	syscall_set_handler(kernel_syscall_handler);

	//Set the syscall exception priority to the lowest possible;
	syscall_set_priority(KERNEL_PREMPTION_PRIORITY);

	//Enable the syscall exception;
	syscall_enable();
	 */
	
	
	/*
	 * Start execution;
	 */
	
	//Log;
	kernel_log_("\nKernel initialisation sequence complete. Entering thread mode ...\n");
	
	
	//Enter thread mode and un-privilege, provide the kernel stack for interrupt handling;
	//Interrupts will be enabled at the end of the function;
	proc_enter_thread_mode(&exception_stack);
	
}


//-------------------------------------------------- Context Switching -------------------------------------------------

/**
 * kernel_get_new_stack : called by threads to get a new stack pointer, providing the current one, and the index of the
 * 	thread;
 * @param thread_id : the thread's index;
 * @param sp : the previous stack pointer;
 * @return the the new stack pointer;
 */

void *proc_switch_context(void *volatile sp) {
	
	//Save process stack pointer;
	sched_set_prc_sp(sp);
	
	//TODO
	//TODO IN SYSCALL;
	
	//If the process is terminated :
	if (prc_process_terminated) {
		
		//Terminate the current process;
		sched_terminate_prc();
		
		//Clear termination flag;
		prc_process_terminated = false;
		
	}
	
	//Commit changes to the scheduler;
	sched_commit();
	
	//Update the duration until next preemption;
	sysclock_set_process_duration(sched_get_req()->activity_time);
	
	
	//Return the appropriate stack pointer;
	return sched_get_sp();
	
}


//------------------------------------------------- Stack header --------------------------------------------------

/**
 * register_stack_header_creator : updates the stack header creator function pointer, if it is null;
 *
 * 	If it is not null, the function fails;
 *
 * @param new_creator : the new stack header creation function;
 * @return true if registration completed;
 */

bool register_stack_header_creator(void (*new_creator)(struct proc_stack *)) {
	
	//If the creator is already registered :
	if (stack_header_creator) {
		
		//Log;
		kernel_log_("register_stack_header_creator : already registered.");
		
		//Fail;
		return false;
		
	}
	
	//Register the stack creator;
	stack_header_creator = new_creator;
	
	//Complete;
	return true;
	
}


/**
 * reset_stack_header_creator : resets the function pointer;
 */

void reset_stack_header_creator() {
	
	//Reset the function pointer;
	stack_header_creator = 0;
	
}


/**
 * create_stack_header : if non-null, calls the stack header creator;
 */

static void create_stack_header(struct proc_stack *stack) {
	
	//Cache the function;
	void (*creator)(struct proc_stack *) = stack_header_creator;
	
	//If it is not null :
	if (creator) {
		
		//Call it;
		(*creator)(stack);
		
	}
	
}


//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
//TODO COPROCESSOR CONTEXT SWITCHER
