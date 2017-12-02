/*
  TempControl.h - Part of TRACER

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


#ifndef TRACER_CLOSEDLOOPS_H
#define TRACER_CLOSEDLOOPS_H


#include <Sensors/Thermistors/Thermistors.h>
#include <ControlLoops/PID.h>
#include <Actions/ContinuousActions.h>
#include "TemperatureController.h"
#include <interface.h>
#include <TaskScheduler/TaskScheduler.h>

class TemperatureController {


public:

    //---------------------------Interrupt regulation functions---------------------------

    //The function to initialise the temperature regulation routine
    static void regulation_init();

    //Temperature regulation routine.
    static void temperature_regulation();

    //The function to finalise the temperature regulation routine
    static void regulation_finalisation();


    //---------------------------HotEnds---------------------------

    typedef struct hotend_state_t {

        //Flags

        //Hotend flag
        bool hotend_flag = false;

        //Activity.
        bool enabled_flag = false;

        //Temperatures.
        bool temperature_flag = false;


        //Values

        uint8_t hotend = 0;

        //States.
        bool enabled = false;

        //Temperatures.
        float temperature = 0;

    };


    //Set the current hotends state.
    static task_state_t set_hotends_state(hotend_state_t state);

    //Scheduler for the state setter
GENERATE_SCHEDULER(set_hotends_state, 0, hotend_state_t, state);

    //Get the current hotends state.
    static hotend_state_t get_hotends_state(uint8_t hotend_id);

private:

    //Hotends States (enabled, disabled)
    static bool *const hotends_enabled;

    //Hotends Temperatures
    static float *const hotends_temps;


    //---------------------------HotBed---------------------------

public:

    typedef struct hotbed_state_t {

        //Flags
        bool enabled_flag = false, temperature_flag = false;

        //Values
        bool enabled = false;
        float temperature = 0;

    };

    //Set the current hotbeds state.
    static task_state_t set_hotbed_state(hotbed_state_t state);

    //Scheduler for the state setter
GENERATE_SCHEDULER(set_hotbed_state, 0, hotbed_state_t, state);

    //Get the current hotbeds state.
    static hotbed_state_t get_hotbeds_state();


private:
    static hotbed_state_t hotbed_state;


    float get_hotend_temperature(uint8_t hotend);

    float get_hotbed_temperature();
};


#endif //TRACER_CLOSEDLOOPS_H