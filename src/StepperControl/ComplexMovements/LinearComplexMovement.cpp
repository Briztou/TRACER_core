/*
  LinearComplexMovement.cpp - Part of TRACER

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


#include "../../config.h"
#include "../../interface.h"

#ifdef ENABLE_STEPPER_CONTROL


#include "LinearComplexMovement.h"
#include "../../Interfaces/TreeInterface/TreeInterface.h"
#include "../../Actions/ContinuousActions.h"
#include "../MovementExecuter.h"
#include "../SpeedManager.h"
#include "../SpeedPlanner.h"
#include "../../Core/EEPROMStorage.h"
#include "../StepperController.h"
#include "../mathProcess.hpp"
#include "../motion_data.h"
#include "../TrajectoryExecuter.h"


#define PROCESSING_STEPS (uint8_t)7

/*
 *  Procedure :
 *  Calcul des positions de depart et d'arrivee
 *  Calcul du premier mouvement
 *  calcul du dernier mouvement
 *
 *
 */



/*
 * Prepare_motion : takes a float[NB_AXIS] in argument.
 *
 *      It goes to the specified coordinates, by tracing a line in the high level coordinates
 *
 */

void LinearComplexMovement::prepare_motion(const float *destinations_t) {

    //Distances array

    uint32_t absolute_distances[NB_STEPPERS]{0};

    //Extract motion distances, and set greater axis to 0.
    uint8_t max_axis = setup_movement_data(destinations_t, absolute_distances);

    //null enqueue_movement, nothing to do.
    if (max_axis == 255)
        return;

    //Move choice : a m
    if (absolute_distances[max_axis] < PROCESSING_STEPS) {
        //max distance < steps per elementary-move -> only one micro enqueue_movement
        //TODO micro_move(absolute_distances);
    } else {
        //A enqueue_movement is indexed on an int value -> max distance must not be > INT_OVF
        //max distance < max value for int : only one enqueue_movement
        enqueue_movement(absolute_distances);
    }

}

/*----------------------------------------------------------------------------------------------------------------------
------------------------------------------------PREPARATORS-------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------*/
/*
 * Parameter set function. It :
 * - extracts distances;
 * - sets steppers directions
 * - converts distances to absolute values
 * - set speed according to the movement (movement dimensions and greater axis)
 */

uint8_t LinearComplexMovement::setup_movement_data(const float *destinations_t, uint32_t *absolute_distances) {

    //As distance on one axis can be null, an axis axis variable must be introduced. incremented after axis processed

    float distsmm[NB_STEPPERS]{0};


    //TODO USE MOTION_SIZE PROVIDED BY THE PARSER

    float sq_dist_sum = 0;
    sig_t direction_signature = 0;
    //uint8_t max_axis = 255;
    //uint32_t max_dist = 0;

    //Order determination : Using an insertion sort on end_distances, and extract axis_data at the same time
    for (uint8_t axis = 0; axis < NB_STEPPERS; axis++) {
        //float steps;
        //int32_t steps_destination, distance;

        steps- = EEPROMStorage::steps[axis];
        steps_destination = (int32_t) (destinations_t[axis] * steps);
        distance = steps_destination - SpeedPlanner::positions[axis];

        //If distance is not null :
        if (distance) {

            //Set distance
            dists[axis] = distance;

            //Update position
            SpeedPlanner::positions[axis] = steps_destination;

            //Update end distances
            disable_stepper_interrupt()

            SpeedManager::end_distances[axis] += distance;

            enable_stepper_interrupt()

            //Millimeter relative distances computation
            float relative_distances_mm = distsmm[axis] = (float) distance / steps;
            sq_dist_sum += relative_distances_mm * relative_distances_mm;

            /*
             * Direction : builds the direction signature :
             * binary where the nth msb is 0 if axis n is positive, 1 if direction is negative
             */

            if (distance < 0) {
                distance = -distance;
                direction_signature |= SpeedPlanner::axis_signatures[axis];
            }

            //Take the absolute distance, and compare if it is the greatest distance. If true, memorise the max axis.
            uint32_t absolute_distance = (uint32_t) distance;
            if (absolute_distance > max_dist) {
                max_dist = absolute_distance;
                max_axis = axis;
            }

            absolute_distances[axis] = absolute_distance;

        }
    }

    //If null enqueue_movement : fail with error value;
    if (max_axis == 255)
        return 255;

    //Direction memorising :
    data_to_fill.negative_signatures = direction_signature;

    data_to_fill.max_axis = max_axis;

    //Local variable for speed datacomputation
    float sqrt_sq_dist_sum = (float) sqrt(sq_dist_sum);
    float distance_coefficient = distsmm[max_axis] / sqrt_sq_dist_sum;


    //Get the adjusted regulation speed;
    float regulation_speed = SpeedPlanner::get_adjusted_regulation_speed_linear(distsmm, sqrt_sq_dist_sum);

    //Calculate and fill the speed data
    SpeedPlanner::pre_set_speed_axis(max_axis, distsmm, sqrt_sq_dist_sum, dists, regulation_speed, PROCESSING_STEPS);

    TrajectoryExecuter::fill_processors(initialise_motion, StepperController::fastStep, process_position, process_speed);

    return max_axis;

}


void LinearComplexMovement::step_and_delay(uint8_t sig) {
    //simple_delay(delay0);
    StepperController::fastStep(sig);
}

/*----------------------------------------------------------------------------------------------------------------------
---------------------------------------------------MOVES----------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------*/

void LinearComplexMovement::micro_move(uint32_t *dists) {//TODO FOX PASSER EN ELEMENTARY_DISTS
    /*
    //Setting destination
    for (uint8_t axis = 0; axis < dimension; axis++)
        next_t[axis] = (uint16_t) dists[axis];

    //Effective StepperControl.
    elementary_motion(0);
     */
}

void LinearComplexMovement::enqueue_movement(uint32_t *dists) {

    //Position process preparation : set slopes and shift_nb
    set_position_data(dists);

    //motion data setting
    set_motion_data(dists);

    //Enqueue motion_data
    data_queue.push(data_to_fill);

    //Enqueue motion_data
    TrajectoryExecuter::enqueue_movement_data();


}

/*
 * The position processing init function : it sets parameters required to process position :
 *  - slopes
 *  - shift_nb
 */
void LinearComplexMovement::set_position_data(uint32_t *dists) {

    //max_axis must have been defined previously

    uint8_t max_axis = data_to_fill.max_axis;

    float *slopes = data_to_fill.slopes;

    slopes[max_axis] = 1;

    float max_dist = dists[max_axis];

    //Determine all slopes
    for (uint16_t axis = 0; axis < NB_STEPPERS; axis++) {
        if (axis != max_axis) {
            slopes[axis] = (float)dists[axis]/max_dist;
        }
    }
}

/*
 * set_motion_data : this function fills movement data on the motion_data_to_fill
 *
 * It computes count, and the first elementary distances.
 *
 * Afterwards, it calls MotionExecuter::fill_movement_data, that hashes the first sub_movement, and fills data.
 */
void LinearComplexMovement::set_motion_data(uint32_t *motion_dists) {

    uint32_t count = motion_dists[data_to_fill.max_axis] / PROCESSING_STEPS;

    sig_t nsig = data_to_fill.negative_signatures;

    const uint8_t max_axis = data_to_fill.max_axis;
    const float *slopes = data_to_fill.slopes;
    uint8_t *const f_pos = data_to_fill.first_pos;

    //Initial elementary distances
    uint8_t elementary_dists[NB_STEPPERS];

    for (uint16_t axis = 0; axis < NB_STEPPERS; axis++) {
        uint8_t p;
        if (axis == max_axis) {
            p = (uint8_t) PROCESSING_STEPS;
        } else {
            p = (uint8_t) (slopes[axis] * PROCESSING_STEPS);
        }
        f_pos[axis] = elementary_dists[axis] = p;
    }

    //Fill beginning signatures
    TrajectoryExecuter::fill_movement_data(true, elementary_dists, count, nsig);

    uint32_t last_pos_max = count*PROCESSING_STEPS;

    //Initial elementary distances
    for (uint16_t axis = 0; axis < NB_STEPPERS; axis++) {
        if (axis==max_axis) {
            elementary_dists[axis] = (uint8_t) (motion_dists[axis] - last_pos_max);
        } else {
            elementary_dists[axis] = (uint8_t) (motion_dists[axis] - (uint32_t)(slopes[axis] * last_pos_max));
        }
    }

    //Fill ending signatures
    TrajectoryExecuter::fill_movement_data(false, elementary_dists, count, nsig);
}

void LinearComplexMovement::initialise_motion() {
//TODO COPIED... BETTER TAKE DIRECTLY POINTER

    linear_data data = data_queue.pull();

    MR_max_axis = data.max_axis;

    MR_negative_signatures = data.negative_signatures;

    for (int axis = 0; axis<NB_STEPPERS; axis++) {
        MR_positions[axis] = (uint32_t) data.first_pos[axis];
    }

    memcpy(MR_slopes, data.slopes, 4 * NB_STEPPERS);

}

/*
 * Position processing function.
 * It takes 2*dimension-1 processing windows to determine all positions
 */
sig_t LinearComplexMovement::process_position(uint8_t *elementary_dists) {//2n-2


    uint32_t i1 = (MR_positions[MR_max_axis] += (elementary_dists[MR_max_axis] = PROCESSING_STEPS));
    uint32_t i2;

#define STEPPER(i, ...) \
    if ((uint8_t)i!=MR_max_axis){\
        i2 = (uint32_t) (MR_slopes[i] * i1);\
        MR_positions[i] += (elementary_dists[i] = (uint8_t) ((i2 - MR_positions[i])));\
    }\

#include "../../config.h"

#undef STEPPER



    return MR_negative_signatures;
}

/*
 * The Speed processing function : it sets the period of the stepper interrupt
 */
void LinearComplexMovement::process_speed() {
    set_stepper_int_period(SpeedManager::delay0);
}


#define m LinearComplexMovement

Queue<linear_data> m::data_queue(MOTION_DATA_QUEUE_SIZE);
linear_data m::data_to_fill;

uint32_t *const m::MR_positions = new uint32_t[NB_STEPPERS];
float *const m::MR_slopes = new float[NB_STEPPERS];
uint8_t m::MR_max_axis;
sig_t m::MR_negative_signatures;

#undef m;

#endif