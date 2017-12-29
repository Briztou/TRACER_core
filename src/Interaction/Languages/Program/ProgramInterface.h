/*
  TreeInterface.h - Part of TRACER

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
#ifdef ENABLE_PROGRAM_INTERFACE

#include <hardware_language_abstraction.h>
#include <DataStructures/ArgumentsContainer.h>
#include <TaskScheduler/task_state_t.h>

#if !defined(HL_SERIAL_FLAG)
#error Program requires Serial. If your board and language supports those, enable them in file "hardware_language_abstraction.h"
#endif

#ifndef CINTERFACE
#define CINTERFACE

#define PI Program

#include "Config/program_interface_config.h"



//TODO POTENTIAL LOSS OF PACKETS : The data size transmitted is not the real size, but the size with BEGIN_BYTES doubled

/*
 * The Program class is in charge of receiving data from the outside, and to decode them, according to
 *      the syntax specified in "interface_config.h" (More description about the syntax itself is in the config_files file).
 *
 * When it received a command, it enqueues it in TaskScheduler's external_tasks queue.
 */

class Program {

public :

    //Initialise the interface
    static void init();

    //Initialise the command aliases
    static void initialise_aliases();

    //read_data data on the communication layer
    static void read_data();

    //Message emission
    static void send_packet();

    //Tree Structure update
    static task_state_t send_tree_structure();

    //Add a task : create the struct to contain dynamic_args data, save it, and send to task to the scheduler.
    static void add_task(task_state_t (*task)(void *), char *command, uint8_t size);


    //------------------------------------------------Packet Preparation------------------------------------------------

public :

    //System packet preparation
    static void prepare_system_packet();

    //EEPROMMap packet preparation
    static void prepare_EEPROM_packet();

private :

    static void prepare_structure_packet();


    //---------------------------------------------System commands aliases----------------------------------------------

public :

    //display data
    static void echo(string_t msg);

    //send the position
    static void send_position(float *t);


    //-------------------------------------------------Emission methods-------------------------------------------------

public :
    //Outgoing data reset;
    static void prepare_data_out(const char *command_id, uint8_t command_id_size);


    //Methods to append data to the current output message

    //Append a char
    static void add_char_out(char data);

    //Append an int
    static void add_int_out(int data);

    //Append a long
    static void add_int32_t_out(int32_t data);

    //Append an float
    static void add_float_out(float data);

    //Append an String
    static void add_string_out(const char *data);


    //---------------------------------Functions called by ProgramInterfaceCommands------------------------------

public :

    //Get a particular argument in the storage
    static char *get_arguments(uint8_t task_index, uint8_t *size);

    //Mark a task as executed
    static void validate_task(uint8_t task_index);


    //----------------------------------------Emission and Reception data fields----------------------------------------

private:


    /*
     * For input, and output data, a fixed space is allocated, to store the current data.
     *
     * For the emission data, it is structured like the following
     *
     *  0  :  1   :   2    : -> : DATA_SIZE + 1
     * 255 : size :  data    ->
     *
     *
     * And for the reception data, it is structured like the following
     *
     *  0  :  1   :   2    : -> : DATA_SIZE - 1
     * data    ->
     *
     */

    //Emission data : pointer to the end of the message, message origin, and size
    static char *data_out;
    static char *const data_out_0;
    static char *const size_ptr;
    static uint8_t data_out_size;


    //Reception data : pointer to the end of the message, message origin, and size
    static char *data_in;
    static char *const data_in_0;


    //Flag set if a command is being parsed.
    static bool parsing_began;

    //Flag set if a BEGIN_CHAR has just been detected
    static bool first_detected;

    static uint8_t in_data_remaining;
    static uint8_t in_data_size;

    static ArgumentsContainer arguments_storage;

    //Reset incomming data, to receive new packets properly
    static void reset_input_data();

    //Incomming message processing
    static void process(char *command, uint8_t size);


    //Command aliases

#define GO_UPPER
#define GO_LOWER(i, name)
#define CREATE_LEAF(i, name)\
    private :\
    static uint8_t name##_size;\
    static char *const name##_id;\
    public :\
    static void send_##name();


#include <Config/program_interface_config.h>


};

#endif

#endif