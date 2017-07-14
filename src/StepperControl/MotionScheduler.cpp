/*
  MotionScheduler.cpp - Part of TRACER

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
  along with TRACER.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "../config.h"

#ifdef ENABLE_STEPPER_CONTROL

#include "MotionScheduler.h"
#include "StepperController.h"
#include "MotionExecuter.h"

#include "../Interfaces/CommandInterface.h"
#include "../Actions/ContinuousActions.h"
#include "../Core/EEPROMStorage.h"


void MotionScheduler::begin() {

#define CARTESIAN_GROUP(i, s0, s1, s2, ms)\
    theorical_regulation_speeds[i]= ms;\
    speed_groups_signatures[i] = ((s0!=-1) ? 1<<s0 : 0)|((s1!=-1) ? 1<<s1: 0)|((s2!=-1) ? 1<<s2 : 0);


#include "../config.h"

#undef CARTESIAN_GROUP

    for (unsigned char axis = 0; axis < NB_STEPPERS; axis++) {
        axis_signatures[axis] = (unsigned char) (1 << axis);
    }


}


void MotionScheduler::send_position() {

#ifdef position_log
    StepperController::send_position();

#else
    CI::echo("POSITION");
    float t[NB_STEPPERS];
    for (int axis = 0; axis < NB_STEPPERS; axis++) {
        t[axis] = (MotionScheduler::positions[axis] / EEPROMStorage::steps[axis]);
    }
    CI::send_position(t);
#endif

}

void MotionScheduler::set_speed_group(unsigned char speed_group) {
    MotionScheduler::speed_group = speed_group;
}


//TODO CHECK MAX_SPEED
void MotionScheduler::set_speed_for_group(unsigned char group_id, float speed) {
    theorical_regulation_speeds[group_id] = speed;
}


/*
 * get_regulation_speed : this function calculates the real regulation speed, from the theoretical one.
 *
 *  It verifies that the speed on each axis does not overpasses the maximum speed on this axis.
 *
 *  If it does, it determines the ratio to reduce the speed, for each axis where the speed is too high.
 *
 *  It applies the maximum ratio.
 */

//TODO PROCESS INV_SQUARE_DIST_SUM HERE, WHERE SPEED_GROUPS SIGNATURES ARE KNOWN
float MotionScheduler::get_regulation_speed(float *const distsmm, const float sqrt_square_dist_sum) {

    //Determination of the regulation speed
    unsigned char group_signature = speed_groups_signatures[speed_group];
    float group_coeff = 0;
    float group_speed = theorical_regulation_speeds[speed_group];

#define STEPPER(i, sig, ...)\
    if (group_signature&sig) {\
        group_coeff+=distsmm[i]*distsmm[i];\
    }

#include "../config.h"

#undef STEPPER

    float theorical_regulation_speed;

    if (group_coeff == 0) {//If no axis in the speed group has to move : find the first speed group with a moving axis.
        CI::echo("ERROR : THE SELECTED SPEED_GROUP DOESN'T COVER YOUR MOVEMENT : SELECTING ANOTHER");
        for (int group = 0; group < NB_SPEED_GROUPS; group++) {
            if (group == speed_group) continue;
            group_signature = speed_groups_signatures[group];
            group_coeff = 0;

#define STEPPER(i, sig, ...)\
            if (group_signature&sig) {\
                group_coeff+=distsmm[i]*distsmm[i];\
            }

#include "../config.h"

#undef STEPPER

            if (group_coeff != 0) {
                group_speed = theorical_regulation_speeds[group];
                CI::echo("SELECTED GROUP" + String(group));
                break;
            }
        }
    }

    if (group_coeff == 0) {
        CI::echo("ERROR : THERE IS NO SPEED GROUP CONTAINING ANY AXIS MOVING DURING YOUR MOVEMENT");
        //TODO ERROR
        return 0;
    }

    CI::echo("ssd-sgc " + String(sqrt_square_dist_sum) + " " + String(sqrt(group_coeff)));

    theorical_regulation_speed = group_speed * sqrt_square_dist_sum / sqrt(group_coeff);

    float scoeff = theorical_regulation_speed / sqrt_square_dist_sum;

    float r = 0;
    bool init = true;
    float speed, tms;

    for (unsigned char axis = 0; axis < NB_STEPPERS; axis++) {
        //Maximum speed checking
        if ((speed = (*(distsmm + axis)) * scoeff) > (tms = EEPROMStorage::maximum_speeds[axis])) {//TODO RESTORE AXIS_t
            r = (init) ? tms / speed : min(r, tms / speed);
            init = false;
        }
    }

    float regulation_speed = theorical_regulation_speed;

    CI::echo("theorical : " + String(theorical_regulation_speed));

    if (r != 0) {
        regulation_speed *= r;
    }

    CI::echo("real : " + String(regulation_speed));


    return regulation_speed;

}


/*
 * pre_set_speed_axis : this function is called during the scheduling of a movement.
 *
 *  new_axis is the axis that will be used for delay regulation.
 *
 *  distance_coefficient is the ratio (axis_dist)/sqrt(sum(distances²)), used to project speed on a particular axis
 *
 *  It prepares the speed change, before the currently planned movement is really executes.
 *
 *  It sets three parameters :
 *      - ratio : the distance ratio, used to adapt the speed management to one axis, in linear moves (1 for others);
 *      - tmp_delay_numerator : the delay numerator that will be used during the movement we now plan;
 *      - tmp_regulation_delay : the regulation delay that will be used during the movement we now plan.
 */
void MotionScheduler::pre_set_speed_axis(unsigned char new_axis, float distance_coefficient, float regulation_speed, unsigned char processing_steps) {


    //the acceleration considered is the new axis acceleration
    float acceleration = EEPROMStorage::accelerations[new_axis];
    float steps = EEPROMStorage::steps[new_axis];

    float ratio = steps / distance_coefficient;
    unsigned int tmp_delay_numerator = (unsigned int) (1000000 / sqrt(2 * steps * acceleration));
    unsigned int tmp_regulation_delay = (unsigned int) (1000000 / (steps * distance_coefficient * regulation_speed));

    CI::echo("REGULATION_DELAY : " + String(tmp_regulation_delay));

    MotionExecuter::fill_speed_data(tmp_delay_numerator, tmp_regulation_delay, ratio, processing_steps);

}

//TODO ACCELERATION CHECKING ON OTHER AXIS
//TODO CALCULER (ts *ta ) AU PREALABLE MATRCIELLEMENT

#undef LOCAL_STEP_AND_WAIT


#define m MotionScheduler


//POWER AND SPEED

unsigned char m::speed_group = 0;

float ttrs[NB_SPEED_GROUPS];
float *const m::theorical_regulation_speeds = ttrs;

unsigned char ttgs[NB_SPEED_GROUPS];
unsigned char *const m::speed_groups_signatures = ttgs;

long pos[NB_STEPPERS];
long *const m::positions = pos;


#define STEPPER(i, j, si, st, sp, ac, dp, ps, pd, pinPower, pmi, vi, pma, va) 1<<i,

unsigned char tmp_signature[NB_STEPPERS];
unsigned char *const m::axis_signatures = tmp_signature;

//Fields for theorical_regulation_speed processing :

unsigned char m::linear_tools_nb;

void (*function_array[NB_CONTINUOUS])(float);

void (**m::linear_set_functions)(float) = function_array;




/*

long d0[NB_STEPPERS], d1[NB_STEPPERS];
long *m::d_0 = d0, *m::d_1 = d1;
long *m::end_distances = d0, *m::next_end_distances = d1;

unsigned char do0[NB_STEPPERS], do1[NB_STEPPERS];
unsigned char *m::d_o_0 = do0, *m::d_o_1 = do1;
unsigned char *m::distances_order = do0, *m::next_distances_order = do1;

bool m::is_d_0 = true;
 */



/*
 //Acceleration checking

float *accelerations = EEPROMStorage::accelerations;
float maxValue = 0;
unsigned char m_axis = 0;
float temp_time;
for (unsigned char axis = 0; axis < dimension; axis++) {
    //Maximum acceleration step
    //Dists are in correct order, not accelerations so axis_t must be used
    if ((temp_time = distsmm[axis] / accelerations[axis_t[axis]]) > maxValue) {
        m_axis = axis;
        maxValue = temp_time;
    }
}

if (m_axis != 0) {
    acceleration0 = *distsmm / maxValue;
} else {
    acceleration0 = accelerations[axis_t[0]];
}
 */

#endif