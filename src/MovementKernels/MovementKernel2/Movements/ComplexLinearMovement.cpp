/*
  ComplexLinearMovement.h - Part of TRACER

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


#include <MovementKernels/MovementKernel2/complex_motion_data.h>
#include <MovementKernels/StepperAbstraction.h>
#include <MovementKernels/MovementKernel2/ComplexTrajectoryExecuter.h>
#include <MovementKernels/MovementKernel2/IncrementComputer.h>
#include "ComplexLinearMovement.h"


/*
 * prepare_movement : this function takes a destination position as an argument and prepares a linear movement,
 *      from the current position to the given destination, if the required movement is not a null movement.
 *
 */

void ComplexLinearMovement::prepare_movement(const float *const destination) {

    //get the movement distances
    float distances[NB_AXIS];

    //extract the array case address (more efficient than by-object-push)
    complex_linear_data *d = linear_data_queue.get_push_ptr();

    //get the positions and slopes pointer;
    float *positions = d->offsets;
    float *slopes = d->slopes;

    //define maximum distance and maximum axis, that will be correctly set in the function below.
    float max_distance = 0;
    uint8_t max_axis = 0;

    //Get distance relative variables
    bool null_move = get_distances(positions, destination, distances, &max_axis, &max_distance);

    //end if the machine is already at the destination;
    if (null_move) return;

    //fill the slopes array
    get_slopes(slopes, distances, max_axis, max_distance);

    //update pre_process vars for increment pre_processing
    pre_process_slopes = slopes;
    pre_process_offsets = positions;
    pre_process_max_axis = max_axis;

    //Extract the increment
    float increment = IncrementComputer::extract_increment(get_position, 0, 1, DISTANCE_TARGET);

    //Enqueue the movement in the trajectory executer
    ComplexTrajectoryExecuter::enqueue_movement(0, max_distance, increment, initialise_motion, finalise_motion,  get_real_time_position);

    //Push the local data
    linear_data_queue.push();

    //Terminate
    return;
}

//--------------------------------------------------Pre-Processing--------------------------------------------------


/*
 * get_distances : this function computes the current move's distances.
 *
 * At the same time, it determines the maximum axis, and return "the current move is a null prepare_movement".

 */

bool
ComplexLinearMovement::get_distances(float *position, const float *destination, float *distances, uint8_t *maximum_axis,
                                     float *maximum_distance) {


    //Get the current position
    StepperAbstraction::get_current_position(position);

    //Initialise the maximum axis and maximum distance.
    uint8_t max_axis = 0;
    float max_dist = 0;

    //boolean for null move detection
    bool null_move = true;

    //for each axis
    for (uint8_t axis = 0; axis < NB_AXIS; axis++) {

        //get the distance
        float distance = destination[axis] - position[axis];

        //check if the distance is zero
        if (!distance) {
            break;
        }

        //if distance is not zero, the movement is not null
        null_move = true;

        //Update max_axis and max_dist if needed
        if (distance > max_dist) {
            max_axis = axis;
            max_dist = distance;
        }

        //save distance
        distances[axis] = distance;

    }

    //update maximum_axis and distance with the determined values
    *maximum_axis = max_axis;
    *maximum_distance = max_dist;

    //return if the current movement is null
    return null_move;

}


/*
 * get_slopes : this function determines the slopes on every axis.
 *
 *  The slope on a particular axis is the ratio of the distance on this axis with the maximum distance of all axis.
 *
 */

void ComplexLinearMovement::get_slopes(float *slopes, const float *const distances, const uint8_t max_axis,
                                       const float max_distance) {

    //get the inverse of the maximum distance
    float inv_max_dist = 1 / max_distance;

    //the maximum axis slope is one;
    slopes[max_axis] = 1;

    //determine slopes on other axis.
    for (uint8_t axis = 0; axis < NB_AXIS; axis++) {
        if (axis != max_axis) {
            slopes[axis] = distances[axis] * inv_max_dist;
        }
    }
}

void ComplexLinearMovement::get_position(float indice, float *positions) {

    //cache vars
    const uint8_t max_axis = pre_process_max_axis;
    const float *const offsets = pre_process_offsets;
    const float *const slopes = pre_process_slopes;

    //position for the maximum axis;
    positions[max_axis] = offsets[max_axis] + indice;

    //positions for other axis
    for (int axis = 0; axis < NB_AXIS; axis++) {
        if (axis != max_axis) {
            positions[max_axis] = offsets[axis] + indice * slopes[axis];
        }
    }

}


//-----------------------------------------------Real_time_Processing-----------------------------------------------


void ComplexLinearMovement::initialise_motion() {

    //Get the address of the top element
    complex_linear_data *d = linear_data_queue.peak();

    //Update all real-time data
    real_time_max_axis = d->max_axis;
    real_time_offsets = d->offsets;
    real_time_slopes = d->slopes;

    //Do not discard the current element, of it is likely to be rewritten.

    //The effective discard will be made in the finalisation function below.

}

void ComplexLinearMovement::finalise_motion() {

    //the current element is now used, discard it.
    linear_data_queue.discard();

}

void ComplexLinearMovement::get_real_time_position(float indice, float *positions) {

    //cache vars
    const uint8_t max_axis = real_time_max_axis;
    const float *const offsets = real_time_offsets;
    const float *const slopes = real_time_slopes;

    //position for the maximum axis;
    positions[max_axis] = offsets[max_axis] + indice;

    //positions for other axis
    for (int axis = 0; axis < NB_AXIS; axis++) {
        if (axis != max_axis) {
            positions[max_axis] = offsets[axis] + indice * slopes[axis];
        }
    }

}


//Static declaration - definition :

#define m ComplexLinearMovement

//the data queue
Queue<complex_linear_data> ComplexLinearMovement::linear_data_queue(MOTION_DATA_QUEUE_SIZE);

//Pre_processing data
float t_ppof[NB_AXIS], t_ppsl[NB_AXIS];
float *m::pre_process_offsets = t_ppof;
float *m::pre_process_slopes = t_ppsl;
uint8_t m::pre_process_max_axis = 0;

//Real time data
float t_rtof[NB_AXIS], t_rtsl[NB_AXIS];
float *m::real_time_offsets = t_rtof;
float *m::real_time_slopes = t_rtsl;
uint8_t m::real_time_max_axis = 0;

#undef m