/*
  ComplexTrajectoryExecuter.cpp - Part of TRACER

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

#include <config.h>

#ifdef ENABLE_STEPPER_CONTROL

#include "TrajectoryTracer.h"
#include "IncrementComputer.h"
#include "SubMovementManager.h"
#include <interface.h>
#include <StepperControl/MachineInterface.h>
#include <Actions/ContinuousActions.h>
#include <StepperControl/StepperController.h>
#include <StepperControl/KinematicsCore1/KinematicsCore1.h>
#include <StepperControl/KinematicsCore2/KinematicsCore2.h>
#include <StepperControl/JerkPlanner.h>

//------------------------------------------------movement_queue_management---------------------------------------------

/*
 * The function to start the movement routine. It will run in interrupts while movements are to trace, and then stop.
 *
 *  It first verifies that movement are enqueued,
 *
 *  If there are, it pops the first one, and initialises it.
 *
 *  It then prepare the movement procedure, and starts the movement routine.
 *
 */

void TrajectoryTracer::start() {

    if (!movement_data_queue.available_elements()) {
        //If no movements are in the queue, no need to start.

        //send an error message
        CI::echo("No movements are present in the queue, starting aborted");

        return;
    }

    CI::echo("Movements procedure started.");

    Kinematics::initialise_tracing_procedure();

    //Initialise the end booleans
    stop_programmed = false;
    final_sub_movement_started = false;

    //Initialise the K2Physics for the first movement
    process_next_movement();

    update_real_time_movement_data();


    //Set up the movement procedure
    prepare_first_sub_movement();

    //Setup properly the interrupt procedure;
    setup_stepper_interrupt(prepare_next_sub_movement, (uint32_t) delay_us);

    //Mark the movement procedure as started
    started = true;

    //Start the interrupt sequence.
    enable_stepper_interrupt();

}


/*
 * stop : this function stops the movement procedure, by simply disabling the interrupt.
 *
 *  After disabling the interrupt, it disables the timer, and set the state flag to disables.
 *
 *  If this function is called during a movement, it will be discarded, and won't continue at next start.
 *
 *  It can be called during the emergency procedure, to stop the machine from moving.
 *
 */

void TrajectoryTracer::stop() {

    //Interrupt the movement routing, by stopping the interrupt sequence
    disable_stepper_interrupt()

    //Get the half of the delay_us period
    uint32_t half_delay = (uint32_t) (delay_us / 2);

    //Wait the half of the delay_us period
    delay_us(half_delay);

    //re-disable the movement routine, prevents the case where the previous disable occurred just before the interrupt.
    disable_stepper_interrupt()

    //Disable the stepper timer, now that the interrupt is disabled.
    disable_stepper_timer();

    //Stop all currently enabled tools;
    stop_tools();

    //Mark the movement routine as stopped
    started = false;

    //Enable the movement enqueuing
    movement_queue_lock_flag = false;

    StepperController::send_position();

}


/*
 * enqueue_unauthorised : this function is used by another process, to verify if the movement queue is not locked.
 *      It returns true if the queue is locked, and the enqueuing is not permitted.
 */

bool TrajectoryTracer::enqueue_unauthorised() {
    return movement_queue_lock_flag;
}


/*
 * enqueue_movement : this function adds the movement provided in argument to the motion queue.
 *
 *  The movement is provided in the form of :
 *      - min and max, the index variable minimum and maximum values, for (resp) the beginning and end positions
 *      - increment : the index increment for the first movement;
 *      - movement_initialisation : the movement's in_real_time initialisation function;
 *      - trajectory_function : the function that will be used to compute new positions in_real_time.
 *
 */

bool TrajectoryTracer::enqueue_movement(float min, float max, void (*movement_initialisation)(),
                                                 void (*movement_finalisation)(),
                                                 void(*pre_process_trajectory_function)(float, float *),
                                                 void(*trajectory_function)(float, float *)) {

    if (movement_queue_lock_flag) {

        //Send an error message.
        CI::echo("ERROR : THE MOVEMENT QUEUE WAS LOCKED WHEN THE MOVEMENT WAS PUSHED. THE MOVEMENT WILL BE IGNORED.");

        //Fail
        return false;
    }


    //---------------Movement container pointers Initialisation-----------------

    movement_data_t *current_movement = movement_data_queue.get_input_ptr();
    movement_data_t *previous_movement = movement_data_queue.read_input_ptr_offset(1);


    //---------------kernel-invariant data-----------------

    //Trajectory vars
    current_movement->min = min;
    current_movement->max = max;
    current_movement->trajectory_function = trajectory_function;
    current_movement->pre_process_trajectory_function = pre_process_trajectory_function;

    //Movement initialisation - finalisation
    current_movement->movement_initialisation = movement_initialisation;
    current_movement->movement_finalisation = movement_finalisation;
    current_movement->jerk_point = false;


    //Get the current insertion position in the linear_powers storage
    float *tools_linear_powers = tools_linear_powers_storage + NB_CONTINUOUS * movement_data_queue.push_indice();

    //Get the action signature and fill the linear_powers storage.
    current_movement->tools_signatures = MachineInterface::get_tools_data(tools_linear_powers);



    //---------------Increment computing, kernel invariant-----------------

    //The increment computer will determine the min and max increment.
    if (!IncrementComputer::determine_increments(current_movement)) {
        return false;
    }


    //---------------Kinematics variable data-----------------

    Kinematics::initialise_kinetics_data(current_movement);

    //---------------Speed and Jerk (kernel variable)-----------------

    //we must check the jerk if the routine is started, or if movements are already present in the queue.
    bool jerk_checking = (started) || (movement_data_queue.available_elements());

    //Control the jerk bounds if required, and initialise the jerk control for the next movement
    JerkPlanner::control_and_initialise_jerk(current_movement, previous_movement, jerk_checking);


    //---------------Jerk adjusting----------------------

    if (jerk_checking && previous_movement->jerk_point) {

        //If the movement has been popped since the processing has started
        if (!movement_data_queue.available_elements()) {

            //Load only the jerk data
            Kinematics::load_real_time_jerk_data(previous_movement);

        }
    }


    /*
     * TODO JERK_PROPAGATION
     *
     */

    //---------------Persisting in Queue-----------------



    //Push
    movement_data_queue.enqueue();

    CI::echo("ENQUEUED : " + str(movement_data_queue.available_elements()));

    //Start the movement procedure if it is not already started.
    if (!started) {
        start();
        CI::echo("STARTED");
    }

    return true;

}


/*
 * process_next_movement : this function pops a movement from the movement queue, and initialises :
 *      - the real time processor;
 *      - the movement;
 */

void TrajectoryTracer::process_next_movement() {

    if (movement_data_queue.available_elements()) {

        //Pull the next movement
        movement_data_t *movement_data = movement_data_queue.read_output();

        //Initialise the new movement
        (*(movement_data->movement_initialisation))();

        //update the finalisation function.
        movement_finalisation = movement_data->movement_finalisation;

        //Update the tools indice.
        next_tools_powers_indice = movement_data_queue.pull_indice();

        //Enable the future movement switch.
        movement_switch_flag = true;

        //Trigger the kernel's movement update, and get the number of sub_movements to wait before switching.
        movement_switch_counter = SubMovementManager::update_current_movement(movement_data);

        //Update now the pre_processing data, the real_time will be in the function below.
        Kinematics::load_pre_process_kinetics_data(movement_data);

        //Don't discard the movement struct for instance, it will be done in update_real_time_jerk_environment;


    }

}


/*
 * update_real_time_jerk_environment : this function actualises the jerk environment for the current move
 *      with the provided data.
 *
 */

void TrajectoryTracer::update_real_time_movement_data() {

    movement_data_t *movement_data = movement_data_queue.read_output();

    //------Tools-----K2Physics-

    //Update tool_environment
    update_tools_data(movement_data);


    //------Jerk------

    //Jerk positions
    SubMovementManager::update_jerk_position(movement_data->jerk_position);

    SubMovementManager::display_distances();

    //Jerk environment

    Kinematics::load_real_time_kinetics_data(movement_data);


    //------Clean------

    //leave a space for a future movement_data.
    movement_data_queue.discard();

    //Disable the movement_data switch.
    movement_switch_flag = false;

}


//----------------------------------------SUB_MOVEMENT_PRE_COMPUTATION--------------------------------------------------


void TrajectoryTracer::prepare_first_sub_movement() {

    //Push the first sub_movement (increment pre-processed to be correct)
    SubMovementManager::push_new_position();


    ///Step 1 : Get a new position to reach
    sub_movement_data_t *sub_movement_data = SubMovementManager::read_next_sub_movement();

    //Initialise the step_distances array we will give to the kernel
    uint8_t *sub_movement_distances = sub_movement_data->step_distances;

    //Copy the direction signature in cache.
    saved_direction_signature = sub_movement_data->direction_signature;


    //Step 2 : Update the end_distances with this step_distances array and compute the heuristic step_distances to jerk/end points
    SubMovementManager::update_end_jerk_distances(saved_direction_signature, sub_movement_distances);

    //-------------------Kinematics call-------------------

    //Give the hand to the kernel who will compute the sub_movement_time_us for the sub-movement
    float sub_movement_time_us = Kinematics::compute_us_time_for_first_sub_movement(sub_movement_data);


    //Compute the signatures for the next movement.
    process_signatures(sub_movement_distances, es0);
    is_es_0 = false;

    //Determine the delay_us sub_movement_time_us for the next sub_movement :
    delay_us = (sub_movement_time_us) / (float) trajectory_index;


    //Discard the current sub_movement in the sub-movements queue.
    SubMovementManager::discard();

    //Push as much sub_movements as possible.
    SubMovementManager::fill_sub_movement_queue();



}


void TrajectoryTracer::prepare_next_sub_movement() {

    //Disable the stepper interrupt for preventing infinite call (causes stack overflow)
    disable_stepper_interrupt();


    //-------------------Initialisation-------------------

    //update the current movement data, and get the correct signature container for the current sub_movement;
    //3us 4 steppers, 8us 17 stepper : 1.5 us + 0.37us per stepper
    sig_t *step_signatures = initialise_sub_movement();

    //Step 1 : Get a new position to reach
    sub_movement_data_t *sub_movement_data = SubMovementManager::read_next_sub_movement();

    STEP_AND_WAIT

    //Initialise the step_distances array we will give to the kernel
    uint8_t *elementary_distances = sub_movement_data->step_distances;

    //Copy the direction signature in cache.
    saved_direction_signature = sub_movement_data->direction_signature;

    //Step 2 : Update the end_distances with this step_distances array and compute the heuristic step_distances to jerk/end points
    SubMovementManager::update_end_jerk_distances(saved_direction_signature, elementary_distances);

    STEP_AND_WAIT


    //-------------------Kinematics call-------------------

    //Give the hand to the kernel who will compute the time_us for the sub-movement
    float time_us = Kinematics::compute_us_time_for_sub_movement(sub_movement_data);


    STEP_AND_WAIT


    //-------------------Signature extraction-------------------

    //Compute the step_signatures corresponding to the step_distances given by the kernel.
    process_signatures(elementary_distances, step_signatures);

    //Determine the delay_us time_us for the next sub_movement :
    delay_us = time_us / (float) trajectory_index;

    //Discard the current sub_movement in the sub-movements queue.
    SubMovementManager::discard();

    //If no more pre-process is required
    if (SubMovementManager::movement_processed()) {
        goto exit;
    }

    STEP_AND_WAIT


    //-------------------Sub-movements pre-computation-------------------

    //Process a first movement for the one we made
    SubMovementManager::push_new_position();

    //If no more pre-process is required
    if (SubMovementManager::movement_processed()) {
        goto exit;
    }

    STEP_AND_WAIT

    //Process a second movement, to fill the queue if needes
    SubMovementManager::push_new_position();


    //-------------------Exit-------------------

    exit:
    //Set the light interrupt function to give time_us to background processes
    set_stepper_int_function(finish_sub_movement);

    //Re-enable the stepper interrupt
    enable_stepper_interrupt();
}


/*
 * initialise_sub_movement : sets the delay_us, the trajectory indice, the steppers directions
 *      and returns the pointer to the elementary signatures processed before
 */

sig_t *TrajectoryTracer::initialise_sub_movement() {

    //If the sub_movement is the first of a new movement :
    if (movement_switch_flag) {

        //If we must now change the movement environment :
        if (!(movement_switch_counter--)) {

            //Update the movement environment.
            update_real_time_movement_data();

        }
    }

    //Update the trajectory index
    saved_trajectory_index = trajectory_index;

    //Set the correct direction
    StepperController::set_directions(saved_direction_signature);

    //update the interrupt period with the float delay_us, that is computed to provide the most accurate period.
    set_stepper_int_period(delay_us);

    //save the motion scheme computed previously, so that new values won't erase the current ones
    if (is_es_0) {
        saved_elementary_signatures = es1, is_es_0 = false;
        return es0;
    } else {
        saved_elementary_signatures = es0, is_es_0 = true;
        return es1;
    }

}



/*
 * process_signatures : this function pre_processes a prepare_movement that will be executed later.
 *
 *  It determines the signatures of the movement.
 *
 *  More details about signatures meaning and the elementary movement algorithm can be found in TRACER's documentation.
 *
 */

void TrajectoryTracer::process_signatures(uint8_t *const elementary_dists, sig_t *elementary_signatures) {
    uint8_t motion_depth = 0;
    sig_t pre_signatures[8];

    while (true) {
        sig_t sig = 0;

        //Get signature for current enqueue_movement

#define STEPPER(i, signature, ...) if (*(elementary_dists+i) & (uint8_t) 1) { sig |= signature; }

#include <config.h>

#undef STEPPER

        //If a enqueue_movement is required to branch :
        pre_signatures[motion_depth] = sig;
        motion_depth++;

        bool end_move = true;

        //Step 2 : shift right and check if last enqueue_movement is reached
#define STEPPER(i, signature, ...) if ((*(elementary_dists+i) >>= 1)) { end_move = false; }

#include  <config.h>

#undef STEPPER

        if (end_move) {//if next_move is null
            trajectory_index = trajectory_indices[motion_depth - 1];
            break;
        }
    }

    //Writing axis_signatures in the correct order
    int i = 0;
    for (; motion_depth--;) {
        elementary_signatures[i++] = pre_signatures[motion_depth];
    }

}


//----------------------------------------------Movement_ending---------------------------------------------------------
int k2_position_indice = 4;

/*
 *
 * MOVEMENT SWITCHING PROCEDURE :
 *  After a sub movement :
 *      - if the last position of the current movement hasn't been processed, then start the sub_movement interrupt
 *      - if the last position has been processed, then load a new movement
 *
 *  This procedure comprises the following steps :
 *      - change the trajectory function;
 *      - change the trajectory variables (min, max, index)
 *      - initialise the new movement
 *
 *
 *
 * finish_sub_movement : this function is called repeatedly when the next sub_movement has been fully planned.
 *
 *  The only thing it does is stepping steppers according to the current sub_movement signatures, and end
 *      to leave time for background tasks.
 *
 *  If the current sub_movement is finished, then it programs the processing of the next sub_movement.
 *
 */

void TrajectoryTracer::finish_sub_movement() {

    //Disable the stepper interrupt for preventing infinite call (causes stack overflow)
    disable_stepper_interrupt();

    //Get the correct signature
    StepperController::fastStep(saved_elementary_signatures[trajectory_array[saved_trajectory_index]]);

    //If the current sub_movement is finished
    if (!saved_trajectory_index--) {


        //Position log
#ifdef position_log
        if (!(k2_position_indice--)) {
            Kinematics::send_position();
            k2_position_indice = 4;
        }
#endif


        if (stop_programmed) {
            //if (false) {

            if (movement_data_queue.available_elements()) {
                //If another movement has been pushed just after stop_programmed was set (rare case) :

                //Process the next movement
                process_next_movement();
                //if the routine will stop at the end of the current movement:

                stop_programmed = false;

                final_sub_movement_started = true;

            } else if (final_sub_movement_started) {
                //if the final sub_movement is now executed :

                Kinematics::send_position();

                //Stop the routine
                stop();

                CI::echo("STOPPED");

                return;

            } else if (SubMovementManager::available_sub_movements() == 0) {

                //if the last position has just been popped :

                //engage the last sub_movement
                initialise_sub_movement();

                //Mark the final sub_movement
                final_sub_movement_started = true;

                //re-interrupt on this function, as no more process is required
                enable_stepper_interrupt();

                return;

            } else if (SubMovementManager::available_sub_movements() == 1) {
                movement_queue_lock_flag = true;
            }


        } else if (SubMovementManager::movement_processed()) {

            //If the movement pre-processing is finished :

            //finalise the current movement
            (*movement_finalisation)();

            if (movement_data_queue.available_elements()) {
                //If another movement can be loaded :

                //Process the next movement
                process_next_movement();


            } else {
                //If no more movement have been planned :

                stop_programmed = true;
            }
        }


        //interrupt on the normal routine
        set_stepper_int_function(prepare_next_sub_movement);

    }

    enable_stepper_interrupt();
}


//--------------------------------------------------------TOOLS---------------------------------------------------------


/*
 * update_tools_data : this function changes tools linear speeds, at an effective movement switch.
 *
 *  It stops tools that are not used, and  updates the number of tools, speeds, and update function,
 *
 */

void TrajectoryTracer::update_tools_data(movement_data_t *movement) {

    //Cache for the tools signature
    sig_t next_tools_signature = movement->tools_signatures;

    //determine the tools that must be stopped : those are present in the current signature and not in the next one.
    sig_t stop_signature = current_tools_signature & (~next_tools_signature);

    //Stop these tools
    MachineInterface::stop_tools(stop_signature);

    //Update the tool number and the update functions
    tools_nb = MachineInterface::set_tools_updating_function(next_tools_signature, tools_update_functions);

    //update linear powers
    tools_linear_powers = tools_linear_powers_storage + NB_CONTINUOUS * next_tools_powers_indice;

    //update the tools signature
    current_tools_signature = next_tools_signature;
}


/*
 * stop_tools : this function stops the currently enabled tools
 */

void TrajectoryTracer::stop_tools() {

    //Update each tool power with the value 'regulation_speed * linear_power'
    for (uint8_t action = 0; action < tools_nb; action++) {
        (*tools_update_functions[action])(0);
    }

    current_tools_signature = 0;
}


/*
 * update_tools_powers : this function updates the number of tools, and the action_update functions
 */

void TrajectoryTracer::update_tools_powers(float speed) {

    //Update each tool power with the value 'regulation_speed * linear_power'
    for (uint8_t action = 0; action < tools_nb; action++) {
        (*tools_update_functions[action])(tools_linear_powers[action] * speed);
    }

}


//-----------------------------------------Static declarations - definitions--------------------------------------------

#define m TrajectoryTracer

//Acceleration Fields

bool m::started = false;


bool m::stop_programmed = false;
bool m::final_sub_movement_started = false;
bool m::movement_queue_lock_flag = false;


Queue<movement_data_t> m::movement_data_queue(MOVEMENT_DATA_QUEUE_SIZE);

uint8_t ctraj[255] = {
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
        0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

const uint8_t *const m::trajectory_array = ctraj;

sig_t tces0[8], tces1[8];
sig_t *const m::es0 = tces0, *const m::es1 = tces1;
bool m::is_es_0;

const uint8_t ctti[8]{0, 2, 6, 14, 30, 62, 126, 254};
const uint8_t *const m::trajectory_indices = ctti;

sig_t *m::saved_elementary_signatures = tces0;
uint8_t m::saved_trajectory_index;
uint8_t m::trajectory_index;

sig_t m::saved_direction_signature;

void (*m::movement_finalisation)();

float m::delay_us = 0;

uint8_t m::tools_nb;

void (*k2tf[NB_CONTINUOUS]);

void (**m::tools_update_functions)(float) = (void (**)(float)) k2tf;

float t_a_ls[NB_CONTINUOUS * MOVEMENT_DATA_QUEUE_SIZE];
float *const m::tools_linear_powers_storage = t_a_ls;

float *m::tools_linear_powers;

uint8_t m::next_tools_powers_indice = 0;
sig_t m::current_tools_signature = 0;

bool m::movement_switch_flag = false;
uint8_t m::movement_switch_counter = 0;

#undef m
#endif