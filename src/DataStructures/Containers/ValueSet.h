/*
  ValueSet.h - Part of TRACER

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


#ifndef TRACER_VALUESET_H
#define TRACER_VALUESET_H

#include "ValueContainer.h"

template<typename T>
class ValueSet : public ValueContainer<T> {

    //---------------------------------------------- Initialisation ----------------------------------------------

public:

    //Constructor;
    explicit ValueSet(uint8_t max_size);

    //Destructor;
    virtual ~ValueSet() = default;


    //---------------------------------------- Copy Constructor ----------------------------------------

public:

    //Copy constructor;
    ValueSet(const ValueSet &container) : ValueContainer<T>(container){};

    //Move constructor;
    ValueSet(ValueSet &&container) noexcept : ValueContainer<T>(container){};


    //---------------------------------------- Assignment operator ----------------------------------------

    //Copy assignment operator;
    ValueSet &operator=(const ValueSet &container) {ValueContainer<T>::operator=(container);}

    //Copy assignment operator;
    ValueSet &operator=(ValueSet &&container) noexcept {ValueContainer<T>::operator=(container);};


    //---------------------------------------------- Functions ----------------------------------------------

public:

    //Add a task;
    bool add(T new_element) override;

    //Remove a task by object, return true if it was removed;
    bool remove(T element);

    //Remove a task by object, return true if it was removed, and eventually save its old index;
    bool remove(T element, uint8_t *old_index);

};


/*
 * Constructor calls the parent constructor;
 */

template<class T>
ValueSet<T>::ValueSet(uint8_t max_size) : ValueContainer<T>(max_size) {}


/*
 * add : this method overrides the add method, as a function can't be added twice;
 */

template<class T>
bool ValueSet<T>::add(T new_element) {

    //The index to pass to isElementPresent. Unused;
    uint8_t index = 0;

    //If the element is already present :
    if (ValueContainer<T>::isElementPresent(new_element, &index)) {

        //Fail;
        return false;

    }

    //If it is not present, try to add it;
    return ValueContainer<T>::add(new_element);

}


/*
 * remove : search for the given element in the set, and if it is found, it return true;
 *
 *  If not, return false;
 */



template<class T>
bool ValueSet<T>::remove(T element) {

    //The eventual index to remove;
    uint8_t index = 0;

    //If the element is not present :
    if (!ValueContainer<T>::isElementPresent(element, &index)) {

        //Fail, nothing to remove;
        return false;

    }

    //Remove the element by index;
    ValueContainer<T>::remove(index);

    //An element has been deleted, return true;
    return true;

}


/*
 * remove : search for the given element in the set, and if it is found, save its index, and return true;
 *
 *  If not, return false;
 */

template<class T>
bool ValueSet<T>::remove(T element, uint8_t *old_index) {

    //If the element is not present :
    if (!ValueContainer<T>::isElementPresent(element, old_index)) {

        //Fail, nothing to remove;
        return false;

    }

    //Remove the element by index;
    ValueContainer<T>::remove(*old_index);

    //An element has been deleted, return true;
    return true;

}


#endif //TRACER_VALUESET_H