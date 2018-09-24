//
// Created by root on 9/22/18.
//

#include "kx_mcg.h"

#include <kernel/panic.h>
#include <kernel/log.h>
#include <kernel/debug/debug.h>



/*
 * -------------------------------------------------------- FLL --------------------------------------------------------
 */

/**
 * The Frequency Lock Loop is a hardware component that receives and locks an input frequency between
 * 	31250 and 39062.5 Hz, to generate an output signal whose frequency is proportional to the input frequency;
 *
 * 	The output frequency range can be selected between four ranges (expressed in MHz), low (20-25), mid (40-50),
 * 	mid-high (60-75), and high (80-100);
 *
 * 	The input signal can also be selected, between the OSC external ref and the RTC slow clock;
 *
 * 	If the OSC external reference is selected, its frequency may not be in the required range. To support this case,
 * 	a frequency divider is included, to ensure that the input frequency is in the correct range.
 *
 * 	This divisor must be determined properly after the oscillator is selected;
 *
 * 	The configuration of the FLL comprises two steps :
 * 	- The hardware config, made when the FLL is not engaged; multiplication factor is selected, and clock divisor is
 * 		computed;
 * 	- The selection, made when the FLL is engaged by the MCG. The source is selected, and the actual output frequency
 * 		computed;
 */


//The FLL input frequency bounds;
#define FLL_INPUT_MIN 31250
#define FLL_INPUT_MAX 39062

//The fll reference input frequency;
#define FLL_INPUT_REF 32768

//The tolerance around the fll frequency;
#define FLL_INPUT_REF_TOLERANCE 2

//The FLL output frequency; At init, at FEI mode with precise ref, fll factor = 640;
#define FLL_OUTPUT_INIT (32000 * 640)


/**
 * mcg_configure_fll_input : configures the input source of the FLL and determines the consequent input frequency;
 *
 * 	If the external reference is selected in input, a check will be made on if the oscillator is enabled, and
 * 	if the provided divider id determines an input frequency inside the PLL working window;
 *
 * 	If the slow internal reference is selected, @eref_divider_id is not used;
 *
 * @param external_ref : set if the external reference sould be used as input;
 * @param eref_divider_id : FRDIV in case of external ref selection;
 */

void kx_fll_configure(struct kx_fll_config config) {
	
	/*
	 * FLL input config : C1
	 */
	
	
	kernel_log("eref : %d", config.external_ref);
	
	
	//If the external reference is selected :
	if (config.external_ref) {
	
		//Cache the divider index;
		const uint8_t divider_id = config.eref_divider_id;
		
		kernel_log("divider id : %d", divider_id);
		
		//If the divider is invalid :
		if (divider_id > 7) {
			
			//Throw a core error;
			kernel_panic("kx_mc.c : mcg_configure_fll_input : invalid divider;");
			
			//Never reached
			return;
			
		}
		
		//Cache C1;
		uint8_t C1 = *MCG_C1;
		
		//clear FRDIV bts;
		C1 &= ~C1_FRDIV;
		
		//Copy FRDIV in C1;
		C1 |= (divider_id << 3);
		
		//Update C1;
		*MCG_C1 = C1;
		
	}
	
	
	
	/*
	 * FLL output config : C4
	 */
	
	//Cache C4, and clear DMX32 and DCO Range;
	uint8_t C4 = *MCG_C4 & ~(C4_DMX32 | C4_DRST_DRS);
	
	//If the exact frequency must be selected :
	if (config.exact_32768_ref) {
		
		//If the input frequency is appropriate :
		
		//Set the DMX32 flag;
		C4 |= C4_DMX32;
		
	}
	
	//Save DCO range Select;
	C4 |= (config.f_range & 3) << 5;
	
	
	kernel_log("C4 : %h", C4);
	
	
	//Write C4;
	*MCG_C4 = C4;
	
}


//Wait till FLL acquires the PLLS clock;
void kx_fll_acquire_plls() {
	
	//Clear the PLLS bit in C6 to select the FLL;
	*MCG_C6 &= ~C6_PLLS;
	
	//While the PLL owns PLLST (S_PLLST set);
	while ((*MCG_S) & S_PLLST);
	
	kernel_log("S : %h", (uint32_t)(*MCG_S));
}


void kx_fll_set_external_clocking() {
	
	kernel_log_("CLEARING C1[IREFS]");
	
	//Clear IREFS;
	*MCG_C1 &= ~C1_IREFS;
	
	//Wait till IREFST is set;
	while((*MCG_S) & S_IREFST) {
		kernel_log("SET %h", *MCG_S);
		debug_delay_ms(100);
	}
}


void kx_fll_set_internal_clocking() {
	
	kernel_log_("SETTING C1[IREFS]");
	
	//Set IREFS;
	*MCG_C1 |= C1_IREFS;
	
	//Wait till IREFST is set;
	while(!((*MCG_S) & S_IREFST));
	
}


struct range_freqs {
	
	const uint32_t min_freq;
	
	const uint32_t max_freq;
	
	const uint16_t fll_factor;
	
	const uint32_t ref_freq;
	
};


#define NB_RANGES 4

static const struct range_freqs range_parameters[NB_RANGES] = {
	{.min_freq = 20000000, .max_freq = 25000000, .fll_factor = 640, .ref_freq = 24000000},
	{.min_freq = 40000000, .max_freq = 50000000, .fll_factor = 1280, .ref_freq = 48000000,},
	{.min_freq = 60000000, .max_freq = 75000000, .fll_factor = 1920, .ref_freq = 72000000,},
	{.min_freq = 80000000, .max_freq = 100000000, .fll_factor = 2560, .ref_freq = 96000000,},
};


static uint8_t find_ranges(uint8_t *const ranges, const uint32_t target_frequency) {
	
	/*
	 * Limits
	 */
	
	//If the target is below the lowest range :
	if (target_frequency <= range_parameters[0].min_freq) {
		
		//Plan evaluation of the first range only;
		*ranges = 0;
		
		//The couple of ranges that target is between has been found, we can stop here;
		return 1;
	}
	
	//If the target is below the lowest range :
	if (target_frequency >= range_parameters[NB_RANGES - 1].max_freq) {
		
		//Plan evaluation of the first range onlt;
		*ranges = NB_RANGES - 1;
		
		//The couple of ranges that target is between has been found, we can stop here;
		return 1;
	}
	
	
	/*
	 * Ranges traversal
	 */
	
	//Evaluate each range;
	for (uint8_t range_id = 0; range_id < NB_RANGES; range_id++) {
		
		
		//Cache range bounds;
		const struct range_freqs freqs = range_parameters[range_id];
		
		//If we found a range whose min frequency is higher than the target (first one found) :
		if (target_frequency < freqs.min_freq) {
			
			//Add the lower range to the array;
			ranges[0] = range_id - (uint8_t) 1;
			ranges[1] = range_id;
			
			//The couple of ranges that target is between has been found, stop here;
			return 2;
			
		}
		
		//If the target is inside the current range :
		if (freqs.max_freq >= target_frequency) {
			
			//Add the range to the array;
			*ranges = range_id;
			
			//The range that comprises the target frequency has been found, stop here;
			return 1;
			
		}
		
	}
	
	
}


//For each clock source, there can be multiple comptible dividers, each one determining a fll input frequency;
struct divider_config {
	
	//The resulting frequency at the input of the FLL;
	uint32_t frequency;
	
	//The divider index;
	uint8_t divider_id;
	
};

/**
 * get_low_range_dividers : determines the compatible divider, and saves it, if it exists;
 * @param osc_freq : the frequency of the input oscillator;
 * @param dividers : the array to store the compatible divider if it exists;
 * @return 1 if a compatible divider exists, 0 if not;
 */

static uint8_t get_low_range_dividers(uint32_t osc_freq, struct divider_config *dividers) {
	
	kernel_log("osc freq  : %d ", osc_freq);
	
	//For each possible divider :
	for (uint8_t divider_id = 0; divider_id < 8; divider_id++) {
		
		//If the resulting frequency is in the FLL input frequency window :
		if ((osc_freq < FLL_INPUT_MAX) && (osc_freq > FLL_INPUT_MIN)) {
			
			kernel_log("fr  : %d, div id : %d ", osc_freq, divider_id);
			
			//Save the divider and its frequency;
			(*dividers).frequency = osc_freq, (*dividers).divider_id = divider_id;
			
			//One divider has been found, return 1;
			return 1;
			
		}
		
		//If not, divide the osc frequency by 2, to take the divider index update into account;
		osc_freq >>= 1;
	}
	
	//If no divider was found, fail;
	return 0;
	
}


/**
 * get_high_range_dividers : determines the number of compatible divider and save them if they exist;
 * @param osc_freq : the frequency of the input oscillator;
 * @param dividers : the array to store compatible dividers if they exists;
 * @return the number of compatible dividers found;
 */

static uint8_t get_high_range_dividers(uint32_t osc_freq, struct divider_config *dividers) {
	
	
	kernel_log("osc freq  : %d ", osc_freq);
	
	//A static array, containing dividers;
	static uint16_t high_dividers[8] = {32, 64, 128, 256, 512, 1024, 1280, 1536};
	
	//The number of compatible dividers found;
	uint8_t nb_dividers = 0;
	
	//For each possible divider :
	for (uint8_t divider_id = 0; divider_id < 8; divider_id++) {
		
		//Determine the related frequency;
		uint32_t input_freq = osc_freq / (uint32_t) high_dividers[divider_id];
		
		
		kernel_log("fr  : %d, div id : %d ", input_freq, (uint32_t) high_dividers[divider_id]);
		
		//If the resulting frequency is in the FLL input frequency window :
		if ((input_freq <= FLL_INPUT_MAX) && (input_freq >= FLL_INPUT_MIN)) {
			
			//Save the divider and its frequency;
			(*dividers).frequency = input_freq, (*dividers).divider_id = divider_id;
			
			//Increment the divider insertion case, and the number of found dividers;
			dividers++, nb_dividers++;
			
			//If the divider is lesser than 1024 (index 6), there is no need to check for other dividers,
			// as a division by 2 necessarily goes outside the fll input window;
			//There can also be at most two dividers;
			if ((divider_id < 6) && (nb_dividers == 2))
				break;
			
		}
		
	}
	
	//Return the number of compatible dividers;
	return nb_dividers;
	
}


/*
 * The FLL has three different sources;
 */

const enum fll_source {
	
	FLL_OSC0,
	
	FLL_OSC1,
	
	FLL_IRC_SLOW
};

/*
 * FLL sources will be evaluated successively.
 *
 * 	The best configuration will be saved
 */

struct fll_source_configuration {
	
	//The origin of the clock source : 0 for OSC0, 1 for OSC1, 2 for IRC_slow.
	enum fll_source clock_source;
	
	//The frequency divider, for clock sources 0 and 1;
	uint8_t osc_divider;
	
	//Flag, set if the exact reference is selected;
	bool exact_ref;
	
	//The range index;
	uint8_t range_id;
	
	//The output frequency, resulting of all previous parameters;
	uint32_t output_frequency;
	
};


/**
 * test_configuration : this function will evaluate a clock source of the FLL, to determine if it can provide an
 * 	output frequency closer than the current one.
 *
 * 	If the clock source is an oscillator (@clock_source 0 xor 1), the best clock divider will be found, and updated in
 * 	@best_config for later config;
 *
 * 	If a better configuration is found, @best_config will be updated, and the related frequency will be returned;
 * 	If the clock source is inappropriate for the FLL, or if all its possible configurations do not beat the current
 * 	best one, 0 will be returned;
 *
 * @return true if a better configuration has been found, false if not;
 */

static bool get_best_source_config(
	const uint32_t target_frequency, uint32_t best_frequency,
	const enum fll_source clock_source, const uint32_t source_freq,
	struct fll_source_configuration *const best_config,
	const uint8_t *const ranges, uint8_t nb_ranges
) {
	
	/*
	 * Dividers determination
	 */
	
	//Compatible dividers must be determined; there can be at most two compatible dividers;
	struct divider_config dividers[2];
	uint8_t nb_dividers;
	
	//If the clock source is the IRC slow :
	if (clock_source == 2) {
		
		//The only divider is 1, and so, does not modify the source frequency;
		(*dividers).divider_id = 1, (*dividers).frequency = source_freq, nb_dividers = 1;
		
	} else {
		//If the clock source concerns an OSC, there can be two compatible dividers;
		
		//Depending on the frequency range, we must check dividers for different values;
		if (source_freq < 32786) {
			nb_dividers = get_low_range_dividers(source_freq, dividers);
		} else {
			nb_dividers = get_high_range_dividers(source_freq, dividers);
		}
		
	}
	
	//If no dividers were found :
	if (!nb_dividers) {
		
		//Signal that no better configuration has been found;
		return false;
		
	}
	
	
	
	/*
	 * Configs evaluation : comparison parameters;
	 */
	
	
	//A flag, set if the current configuration has been improved;
	bool improvement = false;
	
	//The current distance to target;
	uint32_t target_dist = U32_DIST(target_frequency, best_frequency);
	
	//The best divider found;
	uint8_t best_divider = 0;
	
	//The best range found;
	uint8_t best_range = 0;
	
	//Does the best configuration use the exact ref ?
	bool best_uses_exact_ref = 0;
	
	
	/*
	 * Configs evaluation;
	 */
	
	
	kernel_log("dividers : %d, ranges : %d ", nb_dividers, nb_ranges);
	
	//For each possible divider :
	for (uint8_t divider_index = 0; divider_index < nb_dividers; divider_index++) {
		
		//Cache the related input frequency;
		uint32_t input_freq = dividers[divider_index].frequency;
		
		//For each possible range :
		for (uint8_t i = 0; i < nb_ranges; i++) {
			
			//Cache the range index;
			const uint8_t range_index = ranges[i];
			
			/*
			 * Compare config in standard mode;
			 */
			
			kernel_log("in freq : %d ", input_freq);
			
			kernel_log("range index : %d, factor : %d ", range_index, range_parameters[range_index].fll_factor);
			
			//Determine the output frequency, in standard mode;
			uint32_t output_freq = input_freq * (uint32_t) range_parameters[range_index].fll_factor;
			
			kernel_log("out freq : %d, target %d, current %d", output_freq, target_frequency, best_frequency);
			
			//Determine the distance;
			uint32_t range_dist = U32_DIST(output_freq, target_frequency);
			
			kernel_log("rdist : %d ,tdist : %d ", range_dist, target_dist);
			
			//If this config is closer :
			if (range_dist < target_dist) {
				
				//Save the config;
				improvement = true, target_dist = range_dist, best_frequency = output_freq;
				best_divider = dividers[divider_index].divider_id, best_uses_exact_ref = false;
				best_range = range_index;
				
			}
			
			/*
			 * Compare config in ref mode;
			 */
			
			//If the input frequency is close enough to the reference clock :
			if ((input_freq < FLL_INPUT_REF + FLL_INPUT_REF_TOLERANCE) &&
				(input_freq > FLL_INPUT_REF - FLL_INPUT_REF_TOLERANCE)) {
				
				//Determine the output frequency, in ref mode;
				output_freq = (uint32_t) range_parameters[range_index].ref_freq;
				
				//Determine the distance;
				range_dist = U32_DIST(output_freq, target_frequency);
				
				//If this config is closer :
				if (range_dist < target_dist) {
					
					//Save the config;
					improvement = true, target_dist = range_dist, best_frequency = output_freq;
					best_divider = dividers[divider_index].divider_id, best_uses_exact_ref = true;
					best_range = range_index;
					
				}
				
			}
			
			//If we hit the target frequency, no need for further evaluation;
			if (!target_dist) {
				
				goto end;
				
			}
			
		}
		
	}
	
	//As there are nested loops, a label is required for double breaking;
	end:
	
	//If a better configuration has been found, update the optimal configuration
	if (improvement) {
		
		/*
		 * Update source optimal parameters;
		 */
		best_config->clock_source = clock_source;
		best_config->osc_divider = best_divider;
		best_config->exact_ref = best_uses_exact_ref;
		best_config->range_id = best_range;
		best_config->output_frequency = best_frequency;
		
		//Signal that a better configuration has been found;
		return true;
		
	} else {
		
		//Signal that no better configuration has been found;
		return false;
		
	}
	
	
}



/**
 * kx_fll_find_configuration : this function will evaluate all configurations that are potentially better than
 * 	the current one, and determine the best of all.
 *
 * 	If this configuration exists, it will update the mcg optimal configuration struct;
 *
 * 	If a configuration matching the target frequency is found, true will be returned. If not, false will;
 *
 * @param target_frequency : the frequency to target;
 * @param config : the current best configuration struct;
 * @return true if an exact configuration is found, false if not;
 */

bool kx_fll_find_configuration(const uint32_t target_frequency, struct mcg_config *const config) {
	
	//Cache the closest frequency;
	uint32_t best_frequency = config->frequency;
	
	//If the current frequency is null :
	if (!config->frequency) {
		
		//Fail, this function can't be called with an empty config.
		kernel_panic("kx_fll_find_configuration : called with an empty configuration; "
						 "the IRC finder must be called before;");
		
	}
	
	
	/*
	 * Ranges determination;
	 */
	
	//Array to contain relevant ranges; contains one or two ranges;
	uint8_t ranges[2];
	
	//TODO COMPARE DISTANCE HERE, TO REDUCE CALCULATIONS
	//Determine relevant ranges and their number; Will be 1 or 2;
	uint8_t nb_ranges = find_ranges(ranges, target_frequency);
	
	
	/*
	 * Sources evaluation;
	 */
	bool internal_clk;
	
	bool target_reached = false;
	
	//Two flags, set if a better configuration is found on the current evaluation, and for all evaluations;
	bool _improvement, improvement = false;
	
	//The best configuration found;
	struct fll_source_configuration best_config;
	
	
	//This macro will evaluate the improvement flag, and if set, update the best frequency,
	// and eventually jump to hit if the target frequency is reached;
	#define CHECK_IMPROVEMENT\
    if (_improvement) {\
        improvement = true;\
        best_frequency = best_config.output_frequency;\
        if (best_frequency == target_frequency)\
            goto hit;\
    }
	
	
	
	//Evaluate the IRC slow config;
	_improvement = get_best_source_config(target_frequency, best_frequency, FLL_IRC_SLOW, IRC_SLOW_FREQ, &best_config,
										  ranges, nb_ranges);
	
	CHECK_IMPROVEMENT
	
	
	//Evaluate the OSC0 config;
	_improvement = get_best_source_config(target_frequency, best_frequency, FLL_OSC0, osc_frequencies[0], &best_config,
										  ranges, nb_ranges);
	
	CHECK_IMPROVEMENT
	
	
	//Evaluate the OSC1 config;
	//_improvement = get_best_source_config(target_frequency, best_frequency, FLL_OSC1, osc_frequencies[1], &best_config,
										 // ranges, nb_ranges);
	
	//CHECK_IMPROVEMENT
	
	//If no better configuration was found :
	if (!improvement) {
		
		//Do nothing and return;
		return 0;
		
	} else {
		
		
		//If a better configuration is found, but target is not hit, go to setup;
		goto setup;
		
	}
	
	
	//hit : reached when the best frequency equals the target frequency;
	hit:
	target_reached = true;
	
	
	//setup : reached when the best frequency equals the target frequency;
	setup:
	
	//Internal clocking if IRC slow clock is used to clock the FLL;
	internal_clk = (best_config.clock_source == FLL_IRC_SLOW);
	
	//Create the config initializer;
	struct mcg_config new_config = {
		
		//Save the frequency;
		.frequency = best_config.output_frequency,
		
		//Enter FEI if internal clocking, FEE if external clocking;
		.mode = (internal_clk) ? FEI : FEE,
		
		//The IRC will be setup, as the IRC slow is directly routed to the FLL;
		.irc_used = false,
		
		//OSC will be used only if the IRC slow clock is not used;
		.osc_enabled = !internal_clk,
		
		//Update the OSC channel. Will be used only if the oscillator is selected;
		.osc_channel = (best_config.clock_source == FLL_OSC0) ? (uint8_t) 0 : (uint8_t) 1,
		
		//FLL used,
		.fll_used = true,
		
		.fll_configuration = {
			.f_range = best_config.range_id,
			.eref_divider_id = best_config.osc_divider,
			.exact_32768_ref = best_config.exact_ref,
			.external_ref = !internal_clk,
			
		},
		
		//PLL not used,
		.pll_used = false,
		
	};
	
	//Update the mcg configuration;
	*config = new_config;
	
	//Return true if the exact frequency is reached;
	return target_reached;
	
}


