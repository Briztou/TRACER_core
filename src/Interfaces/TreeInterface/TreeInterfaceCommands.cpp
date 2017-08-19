/*
  TreeInterfaceCommands.cpp - Part of TRACER

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

#include <MovementKernels/MovementKernel2/Movements/ComplexLinearMovement.h>
#include <MovementKernels/StepperAbstraction.h>
#include "../../config.h"
#ifdef ENABLE_TREE_INTERFACE

#include "TreeInterfaceCommands.h"
#include "../../interface.h"
#include "../../Core/EEPROMStorage.h"

#define EEPROM_SUBCANAL 1
#define PID_SUBCANAL 2
#define LOOP_SUBCANAL 3
#define ACTION_SUBCANAL 4
#define STEPPER_SUBCANAL 5
#define PARAMETER_SUBCANAL 7


#ifdef MONITOR_CANAL


void TreeInterfaceCommands::system_canal_function(char *data, uint8_t size) {



    if (!(size--)) return;
    char sub_canal = *(data++);

    switch(sub_canal) {
        case 0 :
            TI::send_tree_structure();
            break;
        case 1 :
            EEPROM_system_canal(data, size);
            break;
        case 2 :
            pid_system_canal(data, size);
            break;
        case 3 :
            loop_system_canal(data, size);
            break;
        case 4 :
            actions_system_canal(data, size);
            break;
        case 5 :
            steppers_system_canal(data, size);
            break;
        case 6 ://ECHO. DO NOTHING
            //echo_system_canal(data, size);
            break;
        case 7 :
            parameters_system_canal(data, size);
            break;

        default:break;
    }
    return;
}

#endif


//------------------------------------------------CANAL PROCESS FUNCTIONS-----------------------------------------------


void TreeInterfaceCommands::parameters_system_canal(char *data, uint8_t size) {

#ifdef ENABLE_ASSERV

    delay(50);


    if (!size) return;
    char sub_canal = *data;

    switch(sub_canal) {
        case 0 :
            /* Init case : send_packet data concerning all External Parameters.
             * Packet Structure : indice, name, min, max, unit
             */

#define EXTERNAL_PARAMETER(indice, name, min, max, unit)\
            TI::prepare_system_packet();TI::add_char_out(PARAMETER_SUBCANAL);TI::add_char_out(0);\
            TI::add_char_out(indice); TI::add_string_out(#name); TI::add_float_out(min) ;\
            TI::add_float_out(max); TI::add_string_out(#unit);TI::send_packet();

#include "../../config.h"

#undef EXTERNAL_PARAMETER

        break;
        default:
            break;
    }

#endif

}


void TreeInterfaceCommands::pid_system_canal(char *data, uint8_t size) {

#ifdef ENABLE_ASSERV

    if (!size--) return;
    char sub_canal = *data;
    int pi;
    switch(sub_canal) {

        case 0 :
            /* Init case : send_packet data concerning all External Parameters.
             * Packet Structure : indice, name, min, max, unit
             */

            pi = 0;

#define PID(i, name, kp, ki, kd)\
            TI::prepare_system_packet();TI::add_char_out(PID_SUBCANAL);TI::add_char_out(0);\
            TI::add_char_out(i); TI::add_string_out(#name); TI::add_float_out(EEPROMStorage::kps[pi]);\
            TI::add_float_out(EEPROMStorage::kis[pi]); TI::add_float_out(EEPROMStorage::kds[pi]);TI::send_packet();

#include "../../config.h"

#undef PID
            break;
        default:
            break;
    }

#endif
}


void TreeInterfaceCommands::loop_system_canal(char *data, uint8_t size) {

#ifdef ENABLE_ASSERV

    if (!size) return;
    char sub_canal = *data;

    switch(sub_canal) {
        case 0 :
            /* Init case : send_packet data concerning all External Parameters.
             * Packet Structure : indice, name, min, max, unit
             */

#define LOOP_FUNCTION(indice, name, period_ms)\
            TI::prepare_system_packet();TI::add_char_out(LOOP_SUBCANAL);TI::add_char_out(0);\
            TI::add_char_out(indice); TI::add_string_out(#name); TI::add_int_out(period_ms);\
            TI::send_packet();

#include "../../config.h"

#undef LOOP_FUNCTION

            break;
        default:
            break;
    }

#endif

}

void TreeInterfaceCommands::actions_system_canal(char *data, uint8_t size) {

    if (!size) return;
    char sub_canal = *data;

    switch(sub_canal) {
        case 0 ://The initialisation case : send_packet a packet for each stepper

#define BINARY(i, name, powerPin, enableValue)\
            TI::prepare_system_packet();TI::add_char_out(ACTION_SUBCANAL);TI::add_char_out(0);TI::add_char_out(0);\
            TI::add_char_out(i);TI::add_string_out(#name);TI::send_packet();\

#define CONTINUOUS(i, name, powerPin, maxValue)\
            TI::prepare_system_packet();TI::add_char_out(ACTION_SUBCANAL);TI::add_char_out(0);TI::add_char_out(1);\
            TI::add_char_out(i);TI::add_string_out(#name);TI::add_float_out(maxValue);TI::send_packet();\

#define SERVO(i, name, dataPin, minValue, maxValue)\
            TI::prepare_system_packet();TI::add_char_out(ACTION_SUBCANAL);TI::add_char_out(0);TI::add_char_out(2);\
            TI::add_char_out(i);TI::add_string_out(#name);TI::add_float_out(minValue);TI::add_float_out(maxValue);\
            TI::send_packet();\

#include "../../config.h"

#undef BINARY
#undef CONTINUOUS
#undef SERVO
            break;
        default:
            break;
    }
}


void TreeInterfaceCommands::steppers_system_canal(char *data, uint8_t size) {

#ifdef ENABLE_STEPPER_CONTROL

    if (!size) return;

    char sub_canal = *data;

    int group_size;

    switch(sub_canal) {
        case 0 ://The initialisation case : send_packet a packet for each stepper

#define STEPPER_DATA(i, j, ...)\
            TI::prepare_system_packet();TI::add_char_out(STEPPER_SUBCANAL);TI::add_char_out(0);TI::add_char_out(0);\
            TI::add_char_out(i);TI::add_char_out(j);TI::send_packet();

#include "../../config.h"

#undef STEPPER_DATA

#define CARTESIAN_GROUP(i, s0, s1, s2, s)\
            group_size = (s0!=-1)+(s1!=-1)+(s2!=-1);\
            TI::prepare_system_packet();TI::add_char_out(STEPPER_SUBCANAL);TI::add_char_out(0);TI::add_char_out(1);\
            TI::add_char_out(i);TI::add_char_out(group_size);if (s0!=-1) TI::add_char_out(s0);\
            if (s1!=-1) TI::add_char_out(s1);if (s2!=-1) TI::add_char_out(s2);TI::send_packet();

#include "../../config.h"

#undef CARTESIAN_GROUP

        case 1 :
            break;

        default:
            break;
    }

#endif

}



void TreeInterfaceCommands::EEPROM_system_canal(char *data, uint8_t size) {

    if (!size--) return;
    char sub_canal = *(data++);
    float f;
    switch(sub_canal) {
        case 0 : //The initialisation case : send the EEPROM_structure
            EEPROMStorage::send_structure();
            return;

        case 1 : //Read case
            f = EEPROMStorage::read(data, size);
            TI::prepare_EEPROM_packet();
            TI::add_char_out(1);
            TI::add_float_out(f);
            TI::send_packet();
            return;

        case 2 : //Write case
            f = EEPROMStorage::write(data, size);
            TI::prepare_EEPROM_packet();
            TI::add_char_out(2);
            TI::add_float_out(f);
            TI::send_packet();
            return;

        case 3 : //Save profile
            EEPROMStorage::saveProfile();
            TI::prepare_EEPROM_packet();
            TI::add_char_out(3);
            TI::add_float_out(0);
            TI::send_packet();
            return;

        case 4 : //Restore default profile;
            EEPROMStorage::setDefaultProfile();
            TI::prepare_EEPROM_packet();
            TI::add_char_out(4);
            TI::add_float_out(0);
            TI::send_packet();
        default:
            return;
    }

}



void TreeInterfaceCommands::action(char * dptr, uint8_t size) {


    CI::echo("400");
    float coords[NB_AXIS]{0};
    coords[0] = 100;
    coords[1] = 40;
    coords[2] = 10;
    coords[3] = 40;

    StepperAbstraction::set_speed_group(0);

    StepperAbstraction::set_speed_for_group(0, 200);

    ComplexLinearMovement::prepare_movement(coords);

    CI::echo("EXIT");

}

void TreeInterfaceCommands::home(char * dptr, uint8_t size) {

    //HomingMovement::prepare_movement();

}

#endif

