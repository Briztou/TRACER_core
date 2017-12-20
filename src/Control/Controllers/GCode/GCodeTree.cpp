/*
  GCodeTree.cpp - Part of TRACER

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

#include <Config/control_config.h>

#ifdef ENABLE_GCODE_INTERFACE

#include "GCodeTree.h"

/*
 * Constructor.
 */
GCodeTree::GCodeTree(char name, uint8_t nb_children, task_state_t (*const f)(char *)) :

        //Initialise all fields.
        name(name), nb_children(nb_children), function(f), children(new gcode_child_container_t[nb_children]){

}


/*
 * Destructor : calls itself on every sub_node, terminates.
 */

GCodeTree::~GCodeTree() {

    //For every sub_tree :
    for (uint8_t sub_tree = 0; sub_tree < nb_children; sub_tree++) {

        //Delete the sub_tree;
        delete children[sub_tree].tree;

    }

    //delete the nb_children array
    delete[] children;

}


/*
 * get_child : this function returns a pointer to the child at a given index;
 *
 * It fails if the child has not been allocated.
 */

const GCodeTree *GCodeTree::get_child(uint8_t id, bool *success) const {

    //First, we must check that the required child exists well.
    if (id >= nb_children) {

        //If not, fail and return;
        *success = false;
        return nullptr;

    }

    //The operation will succeed only if the child is allocated.
    *success = children[id].allocated;

    //If the child is allocated
    if (*success) {

        //Return the child;
        return children[id].tree;

    }

    //If the child is not allocated
    return nullptr;
}


/*
 * set_child : this function assigns the child at the given index;
 *
 * It fails if the child has already been allocated.
 */

void GCodeTree::set_child(uint8_t id, GCodeTree *child, bool *success) {

    //First, we must check that the required child exists well.
    if (id >= nb_children) {

        //If not, fail and return;
        *success = false;
        return;

    }

    //The operation will succeed only if the child is not allocated.
    *success = !children[id].allocated;

    //If the child is not allocates
    if (*success) {

        //Allocate the child
        children[id].tree = child;

        //Mark the child as allocated;
        children[id].allocated = true;
    }

}


#endif