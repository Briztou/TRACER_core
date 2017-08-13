/*
  ComplexMovement.cpp - Part of TRACER

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

#ifdef ENABLE_STEPPER_CONTROL

#include "ComplexMovement.h"
#include "../../Interfaces/TreeInterface/TreeInterface.h"
#include "../../Core/EEPROMStorage.h"
#include "../SpeedManager.h"
#include "../mathProcess.hpp"
#include "../MovementExecuter.h"
#include "../StepperController.h"
#include "../SpeedPlanner.h"
#include "../LinearMovement/LinearMovement.h"
#include "../TrajectoryExecuter.h"

/*
*      Trajectory tracing :
*      - Divide the trajectory in consecutive positions, expressed in high level coordinates;
*      - Translate each position into Stepper coordinates;
*      - Go to These positions, using the elementary movement algorithm :
*      - Before beginning each elementary movement, get the greater axis (the axis that moves the most)
*      - get the delay time, projected on this axis.
*      - Make the elementary movement with the delay.
*/

void ComplexMovement::set_initial_positions(const float *initial_positions) {

    float first_move_destinations[NB_STEPPERS];
    memcpy(first_move_destinations, SpeedPlanner::positions, 4 * NB_STEPPERS);

    for (uint8_t indice = 0; indice < movement_size; indice++) {
        uint8_t axis = axis_t[indice];
        first_move_destinations[axis] = initial_positions[axis];
    }

    LinearMovement::prepare_motion(first_move_destinations);

}

void ComplexMovement::set_final_positions(const int32_t *final_positions) {

    for (uint8_t indice = 0; indice < movement_size; indice++) {
        uint8_t axis = axis_t[indice];
        int32_t pos = SpeedPlanner::positions[axis], npos = final_positions[axis];
        SpeedPlanner::positions[axis] = npos;
        SpeedManager::end_distances[axis] += npos - pos;
    }

}

void ComplexMovement::fill_processors(void (*init_f)(), sig_t (*position_f)(uint8_t *)) {
    TrajectoryExecuter::fill_processors(init_f, step, position_f, process_speed);
}

void ComplexMovement::set_speed_parameters_enqueue(uint8_t processing_steps) {
    set_speed_coefficients();
    float dists_array[NB_STEPPERS]{0};
    for (uint8_t indice = 0; indice<movement_size; indice++) {
        dists_array[axis_t[indice]] = 1;
    }
    float regulation_speed = SpeedPlanner::get_adjusted_regulation_speed_linear(dists_array, 1);
    //TODO SpeedPlanner::pre_set_speed_axis(*axis_t, 1, regulation_speed, processing_steps);
    TrajectoryExecuter::enqueue_movement_data();

}

void ComplexMovement::set_speed_coefficients() {
    float invsteps0 = (float) (1.0 / *steps);
    for (uint8_t indice = 1; indice < movement_size; indice++) {
        speed_coeffs[indice] = steps[indice] * invsteps0;
    }
}

/*
 * set_motion_data : extracts and fills the movement data required to enqueue the data :
 *      (initial/final) (movement/dir) signatures, and number of sub movements.
 */
void ComplexMovement::set_motion_data(const float min, const float max, const float step,
                                       void (*get_relative_position)(float, int32_t *)) {

    uint8_t elementary_dists[movement_size];

    //Number of sub_movements
    uint16_t count = (uint16_t) ((max - min) / step);

    //Get initial data (dists and dir signature)
    uint8_t nsig = get_movement_distances(min, min + step, get_relative_position, elementary_dists);

    //Fill initial signatures (movement and dir)
    TrajectoryExecuter::fill_movement_data(true, elementary_dists, count, nsig);

    //Get final data (dists and dir signature)
    nsig = get_movement_distances(count * step, max, get_relative_position, elementary_dists);

    //Fill final signatures (movement and dir)
    TrajectoryExecuter::fill_movement_data(false, elementary_dists, count, nsig);

}

/*
 * get_movement_distance : gets the distance for a movement from point_0 to point_1,
 *      for a curve given by the get_relative_position function.
 *      Returns the direction signature for this movement.
 */
uint8_t ComplexMovement::get_movement_distances(float point_0, float point_1,
                                                       void (*get_relative_position)(float, int32_t *),
                                                       uint8_t *distances) {

    //Arrays to contain position;
    int32_t td0[movement_size], td1[movement_size];

    //Get relative positions for both points
    get_relative_position(point_0, td0), get_relative_position(point_1, td1);

    //Direction signature
    uint8_t nsig = 0;

    //Pointers to position arrays;
    const int32_t *dptr0 = td0, *dptr1 = td1;

    //Get effective distances and signature.
    for (uint8_t indice = 0; indice < movement_size; indice++) {

        //Get relative distance and increment pointers
        int d = (int) (*(dptr1++) - *(dptr0++));

        //Convert to absolute distance, and set bit in signature if dist is negative
        if (d < 0) {
            nsig |= SpeedPlanner::axis_signatures[axis_t[indice]];
            d = -d;
        }

        //Save distance
        *(distances++) = (uint8_t) (d);
    }

    return nsig;
}

void ComplexMovement::step(sig_t sig) {
    uint16_t delay = StepperController::fastStepDelay(sig);
    set_stepper_int_period(delay);
}


void ComplexMovement::process_speed() {

    //speed_processing_1 :
    float inverse = invert(SpeedManager::distance_square_root);
    STEP_AND_WAIT;

    //TODO uint16_t tmpdelay = SpeedManager::delay0 = (uint16_t) (SpeedManager::delay_numerator * inverse);

    /*SpeedManager::delay0 = StepperController::delays[*axis_t] = tmpdelay;

    for (uint8_t i = movement_size; i--;) {
        STEP_AND_WAIT;
        StepperController::delays[axis_t[i]] = (uint16_t) (speed_coeffs[i] * (float) tmpdelay);
    }

     */

    STEP_AND_WAIT;
    return;

}


#define m ComplexMovement


float tstps[NB_STEPPERS];
float *const m::steps = tstps;

float tsc[NB_STEPPERS];
float *const m::speed_coeffs = tsc;

void (*t_fdirs[NB_STEPPERS])(bool dir);

//void (**m::dir_set_functions)(bool dir) = t_fdirs;


/*
void ComplexMovement::checkPositionByStep(float *offsets, uint16_t delay, uint16_t count,
                                           void (*process_position)()) {
    //reset_data_pointers();
    for (uint16_t i = 0; i < count; i++, alpha += increment) {
        process_position();

        float t[movement_size];
        for (int axis = 0; axis < movement_size; axis++) {
            t[axis] = pos_t[axis] / steps[axis] + offsets[axis];
        }

        CI::send_position(t);
        delayMicroseconds(delay);
    }
}


void ComplexMovement::checkPosition(float *offsets, uint16_t d) {
    int32_t c[NB_STEPPERS];

    for (alpha = min; alpha < max; alpha += increment) {
        get_position(alpha, c);
        for (int axis = 0; axis < movement_size; axis++) {
            CI::add_float_out(c[axis] / steps[axis] + offsets[axis]);
        }

        CI::send_position();
        delayMicroseconds(d);
    }
}
 */



//--------------------------------------------Increment_extraction------------------------------------------------------
/*

float
ComplexMovement::extract_increment(float point, const float end, float increment, const uint8_t processing_steps,
                                    const void(*get_position)(float, int32_t *)) {
    const uint8_t ms = movement_size;

    int32_t pos[ms], dest[ms];
    uint16_t dist;
    float tmp;

    increment = adjust_increment(point, increment, processing_steps, get_position);
    point += increment;
    get_position(point, pos);

    while (point + increment < end) {
        get_position(point + increment, dest);

        dist = get_distance(dest, pos);

        if (dist < processing_steps) {
            tmp = (float) (increment / 3.);
            while (dist < processing_steps) {
                increment += tmp;
                get_position(point + increment, dest);
                dist = get_distance(dest, pos);
            }
        }

        point += increment;

        memcpy(pos, dest, ms << 2);
    }

    return increment;
}


float ComplexMovement::adjust_increment(const float point, float increment, const uint8_t processing_steps, const void(*get_position)(float, int32_t *)) {

    uint8_t dist;

    int32_t pos[movement_size], dest[movement_size];
    get_position(point, pos);
    get_position(point + increment, dest);
    dist = get_distance(dest, pos);

    bool lastSign = ((dist) > processing_steps);
    float incr = (float) (increment / 5.);

    //if dist>processing steps, incr must be opposed to increment
    if (lastSign)
        incr *= -1;

    //Dichotomy
    while (dist != processing_steps) {
        //If the state changed :
        if (lastSign ^ (dist > processing_steps)) {
            incr *= -1. / 5.;
            lastSign = (dist > processing_steps);
        }

        increment += incr;
        get_position(point + increment, dest);
        dist = get_distance(dest, pos);
    }

    return increment;

}

uint8_t ComplexMovement::get_distance(int32_t *dest, int32_t *pos) {
    int d;
    uint8_t dist;
    uint8_t maxi = 0;

    for (uint8_t axis = 0; axis < movement_size; axis++) {
        d = (int) (dest[axis] - pos[axis]);
        dist = (uint8_t) ((d < 0) ? -d : d);
        if (maxi < dist) {
            maxi = dist;
        }
    }

    return maxi;
}
*/

#endif