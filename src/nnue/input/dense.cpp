/*
	This file is part of Vajolet.
	Copyright (C) 2013-2018 Marco Belli
	
    Vajolet is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Vajolet is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Vajolet.  If not, see <http://www.gnu.org/licenses/>
*/

#include <cassert>
#include <iostream>
#include "dense.h"

DenseInput::DenseInput(const unsigned int size):Input(size) {
    _in.resize(_size, 0.0);
}

DenseInput::DenseInput(const std::vector<double> v):Input(v.size()), _in(v) {
}

DenseInput::~DenseInput() {}

void DenseInput::print() const {
    for(auto& el: _in) {
        std::cout<< el<< " ";
    }
    std::cout<<std::endl;
}

void DenseInput::set(unsigned int index, double v) {
    assert(index < _size);
    _in[index] = v;
}

const double& DenseInput::get(unsigned int index) const {
    assert(index < _size);
    return _in[index];
}

unsigned int DenseInput::getElementNumber() const {
    return _size;
}

const std::pair<unsigned int, double> DenseInput::getElementFromIndex(unsigned int index) const {
    assert(index < _size);
    tempReply = std::make_pair(index, _in[index]);
    return tempReply;
}

void DenseInput::clear() {
    _in.clear();
    _in.resize(_size, 0.0);
}
