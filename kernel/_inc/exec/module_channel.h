

#ifndef TRACER_CH_NAME_H
#define TRACER_CH_NAME_H


/*
 * This file must be included in the source file of a channel module, whose purpose is to be compiled once
 * 	for each channel;
 *
 * 	It checks that standard channel parameters (MODULE_NAME and CHANNEL_ID) are defined, and errors if not;
 *
 * 	It then defines macro utilities that create global symbols for channel data;
 */

/*If MODULE_NAME or CHANNEL_ID is not present :*/
#if !defined(MODULE_NAME) || !defined(CHANNEL_ID)

#error "MODULE_NAME or CHANNEL_ID not provided; Check your makefile"

/*Define them, so that non makefile IDE allow debug;*/
#define MODULE_NAME NAME

#define CHANNEL_ID 0

#endif

/*---------------------------------------------------- String macro ----------------------------------------------------*/

#define _STR(x) #x

#define STR(x) _STR(x)

/*--------------------------------------------------- Symbol creation --------------------------------------------------*/


/*Concatenate the module name, "_ch_" and the channel id. No argument expansion is made;*/
#define _REF_SYMB(module_name, channel_id) module_name##_##channel_id

/*Expand arguments before concatenating the result;*/
#define REFERENCE_SYMBOL(module_name, channel_id) _REF_SYMB(module_name, channel_id)


/*------------------------------------------------ Specs ref creation ------------------------------------------------*/

/*Create a global symbol aliasing the provided variable. Type must "channel_specs";*/
#define MODULE_REFERENCE_CHANNEL(data_name)\
	extern const struct channel_specs REFERENCE_SYMBOL(MODULE_NAME, CHANNEL_ID) __attribute__((alias(#data_name)));


/*------------------------------------------------ referenced function declaration ------------------------------------------------*/

/*Create the header for a referenced function;*/
#define MODULE_DECLARE_FUNCTION(function, ret, args)\
	ret REFERENCE_SYMBOL(MODULE_NAME, function) args;\



#endif /*TRACER_CH_NAME_H*/
