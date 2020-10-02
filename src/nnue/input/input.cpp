#include <iostream>
#include "input.h"

Input::Input(const unsigned int size): _size(size) {}

Input::~Input() {}

unsigned int Input::size() const {
    return _size;
}


