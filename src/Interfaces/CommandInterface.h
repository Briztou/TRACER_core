/*
  CommandInterface.h - Part of TRACER

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

#include "../hardware_language_abstraction.h"
#if !defined(HL_SERIAL)
#error CommandInterface requires Serial. If your board and language supports those, enable them in file "hardware_language_abstraction.h"
#endif

#ifndef LOGGER
#define LOGGER

#define CI CommandInterface

#include "interface_config.h"



//TODO POTENTIAL LOSS OF PACKETS : The data size transmitted is not the real size, but the size with BEGIN_BYTES doubled

/*
 * The CommandInterface class is in charge of receiving data from the outside, and to decode them, according to
 *      the syntax specified in "interface_config.h" (More description about the syntax itself is in the config file).
 *
 * When it received a command, it enqueues it in Core's external_tasks queue.
 */

class CommandInterface {

public :

    static void begin();

    static void initialise_aliases();

    static void read_serial();

    //Message emission
    static void send_packet();

    //System commands aliases
    static void echo(String msg);

    static void send_position(float *t);

    //Tree Structure update
    static void send_tree_structure();

    //System packet preparation
    static void prepare_system_packet();

private :

    static void prepare_structure_packet();


    //-------------------------------------------------Emission methods-------------------------------------------------
public :
    //Outgoing data reseet;
    static void prepare_data_out(const char *command_id, unsigned char command_id_size);


    //Methods to add data to the current outgoing message

    static void add_char_out(char data);

    static void add_int_out(int data);

    static void add_long_out(long data);

    static void add_float_out(float data);

    static void add_string_out(const char *data);

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
    static unsigned char data_out_size;


    //Reception data : pointer to the end of the message, message origin, and size
    static char *data_in;
    static char *const data_in_0;


    static bool packet_began;
    static bool first_detected;
    static unsigned char in_data_remaining;
    static unsigned char in_data_size;


    //Reset incomming data, to receive new packets properly
    static void flush();

    //Incomming message processing
    static void enqueue(char *command, unsigned char size);


    //Command aliases

#define GO_UPPER
#define GO_LOWER(i, name)
#define CREATE_LEAF(i, name)\
    private :\
    static unsigned char name##_size;\
    static char *const name##_id;\
    public :\
    static void send_##name();


#include "interface_config.h"


    static void prepare_EEPROM_packet();
};

#endif

