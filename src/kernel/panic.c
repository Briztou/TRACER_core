#include <kernel/debug/debug.h>
#include "proc.h"
#include "ic.h"
#include "log.h"

/*
  panic.c Part of TRACER

  Copyright (c) 2018 Raphaël Outhier

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


void kernel_panic(const char *msg) {

	ic_disable_interrupts();

	while (1) {

		//Transmit the message;;
		kernel_log(msg, 0, 0);

		//Wait for one second;
		debug_delay_ms((uint32_t) 1000);

	}

}
