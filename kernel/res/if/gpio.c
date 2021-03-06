//
/* Created by root on 9/13/18.*/
/**/

#include "gpio.h"


/*
 * We must define blank functions to define the neutral gpio if;
 */
static void nm(size_t s) {}
static size_t nr(){return 0;}


/*
 * The set of neutral gpio operations;
 */

static const struct gpio_port_operations neutral_ops = {
	.set = &nm,
	.clear = &nm,
	.toggle = &nm,
	.write = &nm,
	.read = &nr,
};


/*
 * The neutral gpio if. Can be safely copied on a gpio if to safely neutralise all accesses;
 */

const struct gpio_if neutral_gpio_interface = {
	.port_identifier = 0,
	.operations = &neutral_ops,
	.pin_mask = 0,
};

