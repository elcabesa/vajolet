#include <cassert>
#include <iostream>
#include "sparse.h"

SparseInput::SparseInput(const unsigned int size):Input(size) {
}

SparseInput::~SparseInput() {}

void SparseInput::print() const {
    for(auto& el: _in) {
        std::cout<< "{"<<el.first<< ","<<el.second<<"} ";
    }
    std::cout<<std::endl;
}

void SparseInput::set(unsigned int index, double v) {
    assert(index < _size);
    //assert(_in.find(index) != _in.end());
    std::pair <unsigned int, double> p (index, v);
    _in.push_back(p);
}

const double& SparseInput::get(unsigned int index) const {
    assert(index < _size);
    auto it = std::find_if(_in.begin(), _in.end(),
    [&index](const std::pair<unsigned, int>& element){ return element.first == index;} );
    if(it != _in.end()) {
        return (it)->second;
    }
    return _zeroInput;
}

unsigned int SparseInput::getElementNumber() const {
    return _in.size();
}

const std::pair<unsigned int, double> SparseInput::getElementFromIndex(unsigned int index) const {
    assert(index < _in.size());
    return _in[index];
}

void SparseInput::clear() {
    _in.clear();
}
