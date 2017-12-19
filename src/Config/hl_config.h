/*
  hardware_language_abstraction.h - Part of TRACER

  Copyright (c) 2017 Raphaël Outhier

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

//Timer 0 and 1 will count on microseconds, at 1MHz
#define TIMER_0_FREQUENCY 1000000
#define TIMER_1_FREQUENCY 1000000

//Timer 2 and 3 will count on milliseconds, at 1KHz
#define TIMER_2_FREQUENCY 1000
#define TIMER_3_FREQUENCY 1000


/*
 * The stepper timer will be attached to timer zero.
 */

#define stepper_period_to_reload(period) period_to_reload_0(period);
#define set_stepper_int_period(period) set_int_period_0(period);
#define set_stepper_int_reload(reload) set_int_reload_0(reload);
#define enable_stepper_interrupt() enable_interrupt_0();
#define disable_stepper_interrupt() disable_interrupt_0();
#define enable_stepper_timer() enable_timer_0();
#define disable_stepper_timer() disable_timer_0();
#define stepper_int_flag() timer_flag_0()
#define stepper_int_flag_clear() reset_timer_flag_0();
#define set_stepper_int_function(f) set_interrupt_function_0(f);
#define setup_stepper_interrupt(f, period) setup_interrupt_0(f, period);
#define clean_stepper_interrupt() clean_interrupt_0();
#define is_stepper_loop_enabled() is_timer_loop_enabled_1();


/*
 * The servo timer will be attached to timer one.
 */

#define servo_period_to_reload(period) period_to_reload_1(period);
#define set_servo_int_period(period) set_int_period_1(period);
#define set_servo_int_reload(reload) set_int_reload_1(reload);
#define enable_servo_interrupt() enable_interrupt_1();
#define disable_servo_interrupt() disable_interrupt_1();
#define enable_servo_timer() enable_timer_1();
#define disable_servo_timer() disable_timer_1();
#define servo_int_flag() timer_flag_1()
#define servo_int_flag_clear() reset_timer_flag_1();
#define set_servo_int_function(f) set_interrupt_function_1(f);
#define setup_servo_interrupt(f, period) setup_interrupt_1(f, period);
#define clean_servo_interrupt() clean_interrupt_1();
#define is_servo_loop_enabled() is_timer_loop_enabled_1();


/*
 * The Temperature regulation timer will be attached to timer two.
 */

#define loop_0_period_to_reload(period) period_to_reload_2(period);
#define set_loop_0_int_period(period) set_int_period_2(period);
#define set_loop_0_int_reload(reload) set_int_reload_2(reload);
#define enable_loop_0_interrupt() enable_interrupt_2();
#define disable_loop_0_interrupt() disable_interrupt_2();
#define enable_loop_0_timer() enable_timer_2();
#define disable_loop_0_timer() disable_timer_2();
#define loop_0_int_flag() timer_flag_2()
#define loop_0_int_flag_clear() reset_timer_flag_2();
#define set_loop_0_int_function(f) set_interrupt_function_2(f);
#define setup_loop_0_interrupt(f, period) setup_interrupt_2(f, period);
#define clean_loop_0_interrupt() clean_interrupt_2();
#define is_loop_0_enabled() is_timer_loop_enabled_1();



//------------------------------------------------INITIALISATION--------------------------------------------------------


//Initialisation function : set the correct priorities, enable interrupts, and disables timers and interrupts

#define init_interrupts() {\
    SIM_SCGC6 |= SIM_SCGC6_PIT; __asm__ volatile("nop");PIT_MCR = 1;\
    NVIC_SET_PRIORITY(IRQ_PIT_CH0, 0);NVIC_ENABLE_IRQ(IRQ_PIT_CH0);\
    NVIC_SET_PRIORITY(IRQ_PIT_CH1, 16);NVIC_ENABLE_IRQ(IRQ_PIT_CH1);\
    NVIC_SET_PRIORITY(IRQ_PIT_CH2, 32);NVIC_ENABLE_IRQ(IRQ_PIT_CH2);\
    NVIC_SET_PRIORITY(IRQ_PIT_CH3, 48);NVIC_ENABLE_IRQ(IRQ_PIT_CH3);\
    clean_interrupt_0();clean_interrupt_1();clean_interrupt_2();clean_interrupt_2();}


#define hl_init() init_interrupts();

