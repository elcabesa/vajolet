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

/*double& ParalledSparseInput::get(unsigned int index) {
    unsigned int newIndex = index + _number * _size;
    return _si.get(newIndex);
}*/

const double& ParalledSparseInput::get(unsigned int index) const {
    assert(index < _size);
    unsigned int newIndex = index + _number * _size;
    return _si.get(newIndex);
}

void ParalledSparseInput::set(unsigned int, double) {
    assert(false);
    //_si.set(index + _number * _size, v);
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
    unsigned int count = 0;
    unsigned int  n = _si.getElementNumber();
    
    if(n) {
        return _elements[index];
    }
    
    for (unsigned int i = 0; i<n; ++i) {
        auto el = _si.getElementFromIndex(i);
        if(el.first >= _number * _size && el.first < (_number + 1) * _size) {
            if( count == index) {
                el.first -= _number * _size;
                return el;
            }
            ++count;
        }
    }
    tempReply = std::make_pair(index, 0/0);
    return tempReply;
}

void ParalledSparseInput::clear() {
    // this cannot be called for ParalledSparseInput
    assert(false);
}
