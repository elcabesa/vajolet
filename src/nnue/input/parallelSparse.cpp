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

#include "parallelSparse.h"

ParalledSparseInput::ParalledSparseInput(const Input& si, unsigned int number, unsigned int size):Input(size), _si(si), _number(number), _elementNumber(0) {
    assert((number+1)*size <= si.size());
    assert(si.size() % size == 0);
}


ParalledSparseInput::~ParalledSparseInput() {}

void ParalledSparseInput::print() const {
    unsigned int  n = _si.getElementNumber();
    for (unsigned int i = 0; i < n; ++i) {
        auto& el = _si.getElementFromIndex(i);
        if(el.first >= _number * _size && el.first < (_number + 1) * _size) {
            std::cout<< "{"<<el.first<< ","<<el.second<<"} ";
        }
    }
    std::cout<<std::endl;
}

const double& ParalledSparseInput::get(unsigned int index) const {
    assert(index < _size);
    unsigned int newIndex = index + _number * _size;
    return _si.get(newIndex);
}

void ParalledSparseInput::set(unsigned int, double) {
    assert(false);
}

unsigned int ParalledSparseInput::getElementNumber() const {
    if(_elementNumber) { return _elementNumber;}
    unsigned int count = 0;
    unsigned int  n = _si.getElementNumber();
    for (unsigned int i = 0; i < n; ++i) {
        auto el = _si.getElementFromIndex(i);
        if(el.first >= _number * _size && el.first < (_number + 1) * _size) {
            ++count;
            el.first -= _number * _size;
            _elements.push_back(el);
        }
    }
    _elementNumber = count;
    return count;
}

const std::pair<unsigned int, double> ParalledSparseInput::getElementFromIndex(unsigned int index) const {
    assert(index < getElementNumber());    
    return _elements[index];
}

void ParalledSparseInput::clear() {
    // this cannot be called for ParalledSparseInput
    assert(false);
}
