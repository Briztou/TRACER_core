//
// Created by root on 3/25/18.
//



#ifndef TRACER_CONTAINER_CPP
#define TRACER_CONTAINER_CPP

#include <DataStructures/string/string.h>
#include "Container.h"


//--------------------------- Construction ---------------------------

/*
 * Default Constructor : initialises the data to an empty array;
 */

template<typename T>
Container<T>::Container() : data(nullptr), size(0) {}


/*
 * Copy Constructor : copies the data of the given array;
 */

template<typename T>
Container<T>::Container(const Container<T> &src) : Container() {

    //Resize to the given array's size;
    if (resize(src.size)) {

        //If the resizing succeeded, copy the content;
        mmemcpy(data, src.data, sizeof(T) * size);

    }

}


/*
 * Move constructor : moves the data of the given array;
 */

template<typename T>
Container<T>::Container(Container<T> &&src) : data(src.data), size(src.size) {

    //Reset the source's data;
    src.data = nullptr;
    src.size = 0;

}


/*
 * Destructor : free the content of the array;
 */

template<typename T>
Container<T>::~Container() {

    //Free the data array;
    free(data);

}


//--------------------------- Assignment ---------------------------

/*
 * Copy assignment operator : frees our data and copies the other's;
 */

template<typename T>
Container<T> &Container<T>::operator=(const Container<T> &src) {

    //If we try to assign ourselves, nothing to do;
    if (&src == this)
        return *this;

    //Resize to the given array's size;
    if (resize(src.size)) {

        //If the resizing succeeded, copy the content;
        mmemcpy(data, src.data, sizeof(T) * size);

    }

    //Return a pointer to us;
    return *this;

}

/*
 * Move assignment operator;
 */

template<typename T>
Container<T> &Container<T>::operator=(Container<T> &&src) noexcept {

    //Move the size and the data pointer;
    size = src.size;
    src.size = 0;
    data = src.data;
    src.data = 0;

    //Return a pointer to us;
    return *this;


}


//--------------------------- Getters ---------------------------

/*
 * getSize : returns the size of the container;
 */
template<typename T>
uint8_t Container<T>::getSize() {

    return size;

}

//--------------------------- Operations ---------------------------

/*
 * add : insert an element at the given position of the array;
 *
 * This function starts to realloc the array, and if it succeeds, saves the new element;
 */

template<typename T>
bool Container<T>::add(uint8_t index, T element) {

    //If the index is invalid, fail;
    if (index > size)
        return false;

    //If the resizing completed :
    if (resize(size + (uint8_t) 1)) {

        //Cache the shift index;
        T *shift_ptr = data + size - (uint8_t)2;

        //All value has been reallocated, now we must shift value from the insertion index;
        for (uint8_t shift_counts = size - (index + (uint8_t)1); shift_counts--;) {

            //Shift the value;
            *(shift_ptr + 1) = *shift_ptr;

            //Update the the pointer;
            shift_ptr--;

        }

        //Save the element;
        element[index] = element;

        //Complete;
        return true;

    }

    //Fail if the reallocation failed;
    return false;

}


/*
 * add : appends a value to the array;
 */

template<typename T>
void Container<T>::add(T element) {

    //Add the element at the end of the array;
    add(size - (uint8_t)1, element);

}


/*
 * set : sets the
 */
template<typename T>
T Container<T>::set(uint8_t index, T element) {

    //If the index is invalid, fail;
    if (index >= size) {
        return 0;
    }

    //Save the old element;
    T old = data[index];

    //Set the element;
    data[index] = element;

    //Return the old element;
    return old;


}


/*
 * Remove : this function will copy the element at the given index, shift all next elements of one index
 *  in order to remove it, and return it;
 */

template<typename T>
void Container<T>::remove(uint8_t index) {

    //If the index is invalid, fail;
    if (index >= size) {
        return;
    }

    //Copy the element;
    T element = data[index];

    //Declare shift pointers;
    T *shift_to = data + index, *shift_from = shift_to + 1;

    //For every element from the next to the last;
    for(uint8_t shift_ctr = size - index - (uint8_t) 1; shift_ctr--;) {

        //Shift the element;
        *(shift_to++) = *(shift_from++);

    }

    //Resize the array;
    resize(size - (uint8_t)1);

    //Return the removed value;
    return;

}


/*
 * clear : empties the array;
 */

template<typename T>
void Container<T>::clear() {

    //Free the entire array;
    free(data);

    //Reset data;
    data = nullptr;

    //Reset the size;
    size = 0;

}


//--------------------------- Resize ---------------------------


/*
 * Resize : this function will try to reallocate the array to the required size;
 */

template<typename T>
bool Container<T>::resize(uint8_t new_size) {

    //Try to reallocate the array;
    void *ptr = realloc(data, new_size * sizeof(T));

    //If the reallocation failed, fail;
    if (ptr != nullptr) {
        return false;
    }

    //If the reallocation succeeded :

    //Update the size;
    size = new_size;

    //Update the array;
    data = (T*) ptr;

    //Complete;
    return true;

}

#endif //TRACER_CONTAINER_CPP