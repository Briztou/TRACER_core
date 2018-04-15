//
// Created by root on 4/6/18.
//


#include "stdint.h"

#include "kinetis.h"


//The stack element type;
typedef uint32_t stack_element_t;

//The stack pointer type;
typedef stack_element_t *stack_ptr_t;


/*
 * Notice :
 *
 *
 *  ARM Cortex M4 Stack Frame :
 *
 *      ----------- <- Pre IRQ
 *      XPSR
 *      PC
 *      LR
 *      R12
 *      R3
 *      R2
 *      R1
 *      R0--------- <- Post IRQ
 */
/*
 * core_set_thread_stack_pointer : creates a temporary variable to contain the casted stack pointer,
 *  and injects assembly inline to move the data of the temp into psp;
 */

#define core_set_thread_stack_pointer(sp) {\
        uint32_t __temp_var_core_set_thread_stack_pointer__ = (uint32_t) (sp);\
        __asm__ __volatile__ ("msr psp, %0" :: "r" (__temp_var_core_set_thread_stack_pointer__));\
    }


/*
 * core_get_thread_stack_pointer : injects assembly code in order to move the data of psp into the
 *  provided variable -> sp must be an existing variable name;
 */

#define core_get_thread_stack_pointer(sp) {\
        __asm__ __volatile__ ("mrs %0, psp" : "=r" ((uint32_t)sp):);\
    }


/*
 * core_stack_thread_context : injects assembly code in order to stack registers that are not
 *  stacked automatically in the process stack;
 *
 *  It moves psp into r0, and then stacks r4-r7, general purposes registers, and finally
 *      stacks s16- s31, floating point unit registers;
 *
 *  You may notice that PSP remains unchanged after the stacking. This is done with purpose, in order to
 *      allow the processor to unstack properly even if core_unstack_context is called;
 */

#define core_stack_thread_context()\
    __asm__ __volatile__ (\
        "mrs r0, psp \n\t"\
        "stmdb r0!, {r4 - r11}\n\t"\
        "vstmdb r0!, {s16 - s31}"\
    )



/*
 * core_stack_thread_context : injects assembly code in order to stack registers that are not
 *  stacked automatically in the process stack;
 *
 *  It moves psp into r0, and then unstacks r4-r7, general purposes registers, and finally
 *      unstacks s16- s31, floating point unit registers;
 *
 *  As core_stack_thread_context didn't alter PSP, this function doesn't either;
 */

#define core_unstack_thread_context()\
    __asm__ __volatile__ (\
        "mrs r0, psp \n\t"\
        "ldmdb r0!, {r4 - r11} \n\t"\
        "vldmdb r0!, {s16 - s31}"\
    )


/*
 * core_init_stack : this function initialises the unstacking environment, so that the given function will
 *  be executed at context switch time;
 *
 *  It starts by caching the process stack pointer, and stacks the thread functions pointers, and the PSR.
 *
 *  Then, it saves the thread index in R12 (next word);
 *
 *  Finally, it leaves space for empty stack frame and saves the stack pointer;
 */

#define core_init_stack(function, end_loop, thread_index) {\
        uint32_t *__core_init_stack_sp__ = 0;\
        core_get_thread_stack_pointer(__core_init_stack_sp__);\
        *(__core_init_stack_sp__ - 1) = 0x01000000;\
        *(__core_init_stack_sp__ - 2) = (uint32_t) (function);\
        *(__core_init_stack_sp__ - 3) = (uint32_t) (end_loop);\
        *(__core_init_stack_sp__ - 4) = (uint32_t) (thread_index);\
        __core_init_stack_sp__ -= 8;\
        core_set_thread_stack_pointer(__core_init_stack_sp__);\
    }


/*
 * core_get_thread_id : this function will set thread_id to the value sored in r12, where was stored the thread
 *  index in the core_init_stack macro;
 */

#define core_get_thread_id(thread_id)\
    __asm__ __volatile__("mov %0, r12": "=r" (thread_id):)

/*
 * core_trigger_context_switch : sets the pendSV exception isPending flag;
 */

#define core_trigger_context_switch()\
    SCB_ICSR |= SCB_ICSR_PENDSVSET
    /*__asm__ __volatile__(\
        "mrs r0, icsr \n\t"\
        "mov r1, #1 \n\t"\
        "and r0, r0, r1, lsl #28 \n\t"\
        "msr icsr, r0 \n\t"\
    );*/

#define context_switch_state()\
    SCB_ICSR &

/*
 * core_enable_interrupts : injects inline assembly code to disable all interrupts. Fault handling is still enabled;
 */

#define core_enable_interrupts() __asm__ __volatile__("cpsie i")

/*
 * core_disable_interrupts : injects inline assembly code to disable all interrupts. Fault handling is still enabled;
 */

#define core_disable_interrupts() __asm__ __volatile__("cpsid i")


/*
 * core_set_thread_mode : set the processor to use the psp, and stay privileged in thread mode;
 */

//TODO UNPRIVILEGE
#define core_set_thread_mode()\
    __asm__ __volatile__(\
        "mov r4, #2 \n\t"\
        "msr control, r4\n\t"\
    )/* Exec. ISB after changing CONTORL (recommended) */\



#define core_get_stack_pointer(sp)\
    __asm__ __volatile__(\
        "mov %0, sp" : "=r" (sp)\
    )



/*
 * ------------------------------------- Stack bound -------------------------------------
 */

/*
 * core_get_stack_begin : determines the stacks beginning case from the allocated pointer and the size;
 *
 *  In an arm cortex, the stack decreases, and the stack pointer points to the last pushed element;
 */

#define core_get_stack_begin(allocated_pointer, size) ((allocated_pointer) + (size))


/*
 * core_get_stack_begin : determines the stacks end case from the allocated pointer and the size;
 *
 *  In an arm cortex, the stack decreases, and the stack pointer points to the last pushed element;
 */

#define core_get_stack_end(allocated_pointer, size) (allocated_pointer)

/*
 * ------------------------------------- Context switch -------------------------------------
 */

/*
 * core_set_context_switcher : sets the function to be called when a context switch is required;
 */

#define core_set_context_switcher(context_switcher)\
    _VectorsRam[14] = context_switcher;\
    NVIC_SET_PRIORITY(-2, 240);


/*
 * ------------------------------------- Systick -------------------------------------
 */

/*
 * core_start_systick_timer :
 *  - sets the system timer reload value;
 *  - sets the function to execute at undf;
 *  - resets the undf flag;
 *  - enables the timer;
 *
 *  the reload value is determined by :
 *
 *
 *      timer_period_seconds = 1 / F_CPU seconds;
 *      systick_period_seconds = systick_period_ms / 1000;
 *
 *      nb_ticks = systick_period / timer_period = F_CPU * systick_period_ms / 1000
 *
 *      ex : FCPU = 120 E6 , for 1 ms systick period, nb_ticks = 120 E6 / 1000 = 120 E3
 */

#define core_start_systick_timer(systick_period_us, systick_function)\
    _VectorsRam[15] = systick_function;\
    NVIC_SET_PRIORITY(-1, 0);\
    SYST_RVR = (systick_period_us) * ((float) F_CPU / (float) 1000);\
    Serial.println("PERIOD : "+String( (systick_period_us) * ((float) F_CPU / (float) 1000)));\
    SCB_ICSR &= ~SCB_ICSR_PENDSTSET;\
    SYST_CSR |= SYST_CSR_ENABLE;\


















