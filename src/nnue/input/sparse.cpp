#include <cassert>
#include <iostream>
#include "sparse.h"

SparseInput::SparseInput(const unsigned int size):Input(size) {
}

SparseInput::SparseInput(unsigned int size, const std::map<unsigned int, double> v):Input(size), _in(v) {
    using pair_type = decltype(_in)::value_type;
    assert( std::max_element(std::begin(_in), std::end(_in),[] (const pair_type & p1, const pair_type & p2) { return p1.first < p2.first;})->first < size);
}

SparseInput::SparseInput(const std::vector<double> v):Input(v.size()) {
    unsigned int index = 0;
    for(auto& val: v) {
        assert(index < _size);
        _in.insert(std::pair<unsigned int, double>(index++,val));
    }
}

SparseInput::SparseInput(unsigned int size, const std::vector<unsigned int> v):Input(size) {
    for(auto& val: v) {
        assert(val < _size);
        _in.insert(std::pair<unsigned int, double>(val, 1.0));
    }
}

SparseInput::~SparseInput() {}

void SparseInput::print() const {
    for(auto& el: _in) {
        std::cout<< "{"<<el.first<< ","<<el.second<<"} ";
    }
    std::cout<<std::endl;
}

/*double& SparseInput::get(unsigned int index) {
    auto it = _in.find(index);
    if (it != _in.end()) {
        return it->second;
    }
    return _zeroInput;
}*/

void SparseInput::set(unsigned int index, double v) {
    assert(index < _size);
    assert(_in.find(index) != _in.end());
    _in[index] = v;
}

const double& SparseInput::get(unsigned int index) const {
    assert(index < _size);
    auto it = _in.find(index);
    if (it != _in.end()) {
        return it->second;
    }
    return _zeroInput;
}

unsigned int SparseInput::getElementNumber() const {
    return _in.size();
}

const std::pair<unsigned int, double> SparseInput::getElementFromIndex(unsigned int index) const {
    assert(index < _in.size());
    auto it = _in.begin();
    std::advance(it, index);
    return (*it);
}

void SparseInput::clear() {
    _in.clear();
}
