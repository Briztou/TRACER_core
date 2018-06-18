/*
  kinetis_PORT.c Part of TRACER

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


#include "kinetis_PORT.h"



//TODO BITWISE.h

/*
 * SET : will set [data]'s bits that are set to 1 in [mask]. [data] is of size [size];
 */

#define SET(data, mask, size) (data) |= (uint##size##_t)(mask)


/*
 * CLEAR : will clear [data]'s bits that are set to 1 in [mask]. [data] is of size [size];
 */

#define CLEAR(data, mask, size) (data) &= ~(uint##size##_t)(mask)



/*
 * ------------------------------- Hardware constants macros -------------------------------
 */
//TODO PJRC TRIBUTE

#define PORT_PCR_ISF            ((uint32_t)0x01000000)        // Interrupt Status Flag
#define PORT_PCR_TO_IRQC(n)        ((uint32_t)(((n) & 15) << 16))    // Interrupt Configuration
#define PORT_PCR_FROM_IRQC(n)        ((uint32_t)(((n) >> 16) & 15))    // Interrupt Configuration
#define PORT_PCR_LK            ((uint32_t)0x00008000)        // Lock Register
#define PORT_PCR_TO_MUX(n)            ((uint32_t)(((n) & 7) << 8))    // Pin Mux Control
#define PORT_PCR_FROM_MUX(n)            ((uint32_t)(((n) >> 8 ) & 7))    // Pin Mux Control
#define PORT_PCR_DSE            ((uint32_t)0x00000040)        // Drive Strength Enable
#define PORT_PCR_ODE            ((uint32_t)0x00000020)        // Open Drain Enable
#define PORT_PCR_PFE            ((uint32_t)0x00000010)        // Passive Filter Enable
#define PORT_PCR_SRE            ((uint32_t)0x00000004)        // Slew Rate Enable
#define PORT_PCR_PE            ((uint32_t)0x00000002)        // Pull Enable
#define PORT_PCR_PS            ((uint32_t)0x00000001)        // Pull Select


/*
 * ------------------------------- Private headers -------------------------------
 */

//Determine IRQ bits from interrupt type;
uint8_t type_to_IRQ_bits(PORT_interrupt_t type);

//Determine interrupt type from IRQ bits;
PORT_interrupt_t IRQ_bits_to_type(uint8_t bits);


/*
 * ------------------------------- Definitions -------------------------------
 */

void PORT_get_GPIO_output_registers(PORT_t *port, GPIO_output_registers_t *registers) {

    //Cache GPIO data pointer;
    volatile kinetis_GPIO_memory_t *const memory = port->GPIO_data;

    //Update the data register;
    registers->data_register = &memory->PDOR;

    //Update the bits set register;
    registers->set_register = &memory->PSOR;

    //Update the bits clear register;
    registers->clear_register = &memory->PCOR;

    //Update the bits toggle register;
    registers->toggle_register = &memory->PTOR;

}


/*
 * type_to_IRQ_bits : determines the value to set in the IRQ bits of the PCR, depending on the interrupt type;
 */

uint8_t type_to_IRQ_bits(PORT_interrupt_t type) {

    /*
     * Direct from the kinetis datasheet;
     */

    switch (type) {
        case PORT_NO_INTERRUPT :
            return 0;
        case PORT_DMA_RISING_EDGE :
            return 1;
        case PORT_DMA_FALLING_EDGE :
            return 2;
        case PORT_DMA_EDGE :
            return 3;
        case PORT_INTERRUPT_0 :
            return 8;
        case PORT_INTERRUPT_RISING_EDGE :
            return 9;
        case PORT_INTERRUPT_FALLING_EDGE :
            return 10;
        case PORT_INTERRUPT_EDGE :
            return 11;
        case PORT_INTERRUPT_1 :
            return 12;
        default:
            return 0;
    }

}


/*
 * type_to_IRQ_bits : determines the value to set in the IRQ bits of the PCR, depending on the interrupt type;
 */

PORT_interrupt_t IRQ_bits_to_type(uint8_t bits) {

    /*
     * Direct from the kinetis datasheet;
     */

    switch (bits) {
        case 1 :
            return PORT_DMA_RISING_EDGE;
        case 2 :
            return PORT_DMA_FALLING_EDGE;
        case 3 :
            return PORT_DMA_EDGE;
        case 8 :
            return PORT_INTERRUPT_0;
        case 9 :
            return PORT_INTERRUPT_RISING_EDGE;
        case 10 :
            return PORT_INTERRUPT_FALLING_EDGE;
        case 11 :
            return PORT_INTERRUPT_EDGE;
        case 12 :
            return PORT_INTERRUPT_1;
        case 0 :
        default:
            return PORT_NO_INTERRUPT;
    }

}


/*
 * PORT_get_pin_config : retrieves a pin's current configuration (avoid defaults mistakes);
 */

void PORT_get_pin_config(PORT_pin_t *pin, PORT_pin_config_t *config) {

    PORT_t *port = pin->port_data;
    uint8_t bit = pin->bit_index;

    //Declare the configuration register to write; Set the flag bit to clear it by default;
    uint32_t config_register = port->PORT_data->PCR[bit];

    //Get IRQ bits;
    uint8_t IRQ_bits = (uint8_t) PORT_PCR_FROM_IRQC(config_register);

    //Update the interrupt type in the config struct;
    config->interrupt_type = IRQ_bits_to_type(IRQ_bits);

    //Update the interrupt function if the interrupt is enabled;
    if (IRQ_bits) {

        //TODO INTERRUPT FUNCTION;

    }

    //Set the multiplexer channel;
    config->mux_channel = (uint8_t) PORT_PCR_FROM_MUX(config_register);

    //If the direction is output, set it. Otherwise, set input;
    if (port->GPIO_data->PDDR & (uint32_t) 1 << bit) {
        config->direction = PORT_OUTPUT;
    } else {
        config->direction = PORT_INPUT;
    }



    /*
     * Input mode :
     */

    //Evaluate the input mode;
    switch (config->input_mode) {

        //Only the pullup requires the PS bit set;
        case PORT_PULL_UP:
            SET(config_register, PORT_PCR_PS, 32);

            //Both pull-modes require the PE bit set;
        case PORT_PULL_DOWN:
            SET(config_register, PORT_PCR_PE, 32);

            //Hysteresis is not supported, High Impedance is the default mode;
        default:
            break;

    }


    /*
     * Filtering :
     */

    //If the active filtering is enabled :
    if (port->PORT_data->DFER & (uint32_t) (1 << bit)) {

        //Set the type and the filtering length;
        config->input_filter.input_filter = PORT_DIGITAL_FILTERING;
        config->input_filter.filtering_length = (uint8_t) port->PORT_data->DFWR;

    } else {

        //Reset the filtering length;
        config->input_filter.filtering_length = 1;

        //If the passive filtering is enabled, mark it. Otherwise, mark the non filtering sequences_initialised;
        if (config_register & PORT_PCR_PFE) {
            config->input_filter.input_filter = PORT_PASSIVE_FILTERING;
        } else {
            config->input_filter.input_filter = PORT_NO_FILTERING;
        }

    }



    /*
     * Output mode :
     *
     * Set the output mode. The section is left verbose to clearly sequences_initialised that all cases are handled;
     *  Compiler optimised;
     */

    //If the output is in open drain :
    if (config_register & PORT_PCR_ODE) {
        config->output_mode = PORT_OPEN_DRAIN;
    } else if (config_register & PORT_PCR_DSE) {
        config->output_mode = PORT_HIGH_DRIVE;
    } else {
        config->output_mode = PORT_PUSH_PULL;
    }

    //Determine and save the slew rate;
    if (config_register & PORT_PCR_SRE) {
        config->slew_rate = PORT_HIGH_RATE;
    } else {
        config->slew_rate = PORT_LOW_RATE;
    }

}


/*
 * PORT_set_pin_configuration : sets all registers to fit the required configuration;
 *
 *  An invalid configuration generates an error;
 */

void PORT_set_pin_configuration(PORT_pin_t *pin, PORT_pin_config_t *config) {

    PORT_t *port = pin->port_data;
    uint8_t bit = pin->bit_index;

    //TODO ERRORS IN CASE OF BAD CONFIGURATION;

    //Declare the configuration register to write; Set the flag bit to clear it by default;
    uint32_t config_register = PORT_PCR_ISF;

    //Get IRQ bits;
    uint8_t IRQ_bits = type_to_IRQ_bits(config->interrupt_type);

    //Set IRQ bits in the register;
    SET(config_register, PORT_PCR_TO_IRQC(IRQ_bits), 32);

    //Update the interrupt function if the interrupt is enabled;
    if (IRQ_bits) {

        //TODO INTERRUPT FUNCTION;

    }

    //Set the multiplexer channel;
    SET(config_register, PORT_PCR_TO_MUX(config->mux_channel), 32);

    //If the data is received :
    if (config->direction == PORT_INPUT) {

        /*
         * Input mode :
         */

        //Evaluate the input mode;
        switch (config->input_mode) {

            //Only the pullup requires the PS bit set;
            case PORT_PULL_UP:
                SET(config_register, PORT_PCR_PS, 32);

                //Both pull-modes require the PE bit set;
            case PORT_PULL_DOWN:
                SET(config_register, PORT_PCR_PE, 32);

                //Hysteresis and repeater are not supported, High Impedance is the default mode;
            default://TODO ERROR
                break;

        }

        //Clear the appropriate bit in the GPIO direction register;
        CLEAR(port->GPIO_data->PDDR, 1 << bit, 32);


        /*
         * Filtering :
         */

        switch (config->input_filter.input_filter) {

            //If the passive filtering must be enabled :
            case PORT_PASSIVE_FILTERING :

                //Set the passive filtering bit in the config register;
                SET(config_register, PORT_PCR_PFE, 32);
                break;

                //If the digital filtering must be enabled :
            case PORT_DIGITAL_FILTERING :

                //Set the appropriate bit in the Filtering Enable Register;
                SET(port->PORT_data->DFER, 1 << bit, 32);

                //Cache the width of the filter;
                uint8_t filter_width = config->input_filter.filtering_length;

                //Major the width of the filter;
                if (filter_width > 15) filter_width = 15;

                //Set the filter width bits;
                SET(port->PORT_data->DFWR, filter_width, 32);

                //TODO FILTERING IS CONFIGURED PER PORT. MARK IT IN DATA;

                //No filtering is default mode;
            default:
                break;

        }

    } else {

        /*
         * Output mode :
         *
         * Set the output mode. The section is left verbose to clearly sequences_initialised that all cases are handled;
         *  Compiler optimised;
         */

        switch (config->output_mode) {
            case PORT_PUSH_PULL:
                break;//Nothing to do;
            case PORT_HIGH_DRIVE:
                SET(config_register, PORT_PCR_DSE, 32);
                break;
            case PORT_OPEN_DRAIN:
            default:
                //Open drain in default case
                SET(config_register, PORT_PCR_ODE, 32);
                break;
        }

        //If the slew rate is slow, set the appropriate bit;
        if (config->slew_rate == PORT_HIGH_RATE) {
            SET(config_register, PORT_PCR_SRE, 32);
        }

        //Set the appropriate bit in the GPIO direction register;
        SET(port->GPIO_data->PDDR, 1 << bit, 32);

    }

    //Write the configuration register;
    port->PORT_data->PCR[bit] = config_register;

}

