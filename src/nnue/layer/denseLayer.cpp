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
#include <cmath>
#include <fstream>
#include <iostream>

#include "denseLayer.h"
#include "differentialList.h"
#include "featureList.h"
template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
DenseLayer<inputType, inputSize, outputSize>::DenseLayer(std::vector<biasType>* bias, std::vector<weightType>* weight, unsigned int biasScale, unsigned int weightScale, unsigned int outShift):
    _inputSize(inputSize),
    _outputSize(outputSize),
    _biasScale(biasScale),
    _weightScale(weightScale),
    _outShift(outShift),
    _bias(bias),
    _weight(weight),
    _output(outputSize)
{}

template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
DenseLayer<inputType, inputSize, outputSize>::~DenseLayer() {
    //std::cout<<"min "<<_min <<std::endl;
    //std::cout<<"MAX "<<_max <<std::endl;
}

template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
int32_t DenseLayer<inputType, inputSize, outputSize>::propagateOut(const std::vector<inputType>& input, const unsigned int index, unsigned int o) {   
    int32_t out = (*_bias)[o];
    for(unsigned int i = 0; i< _inputSize; ++i) {
        out += input[i] * (*_weight)[index + i];
    }
    return out;
}

template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
void DenseLayer<inputType, inputSize, outputSize>::propagate(const std::vector<inputType>& input) {
    unsigned int index = 0;
    for (unsigned int o = 0; o < _outputSize; ++o) {        
        int32_t out = propagateOut(input, index, o);
        _output[o] = std::min(std::max(out >> _outShift, 0), 127);
        index += _inputSize;
    }

    std::cout<<"-----------------------"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {        
        std::cout<< std::pow(2.0, _outShift) * _output[o] / double(_biasScale) <<std::endl;
    }

    /*std::cout<<"----------------"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        std::cout<<_output[o] /1024.0 <<std::endl;
    }*/

    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, double(_output[o]));
        _min = std::min(_min, double(_output[o]));
    }*/
}

template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
unsigned int DenseLayer<inputType, inputSize, outputSize>::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _outputSize < _weight->size());
    return o * _inputSize + i;

}

#define PRINTSTAT
template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
bool DenseLayer<inputType, inputSize, outputSize>::deserialize(std::ifstream& ss) {
#ifdef PRINTSTAT
    unsigned int count = 0;
    double min = 1e8;
    double max = -1e8;
#endif
    //std::cout<<"DESERIALIZE DENSE LAYER"<<std::endl;
    union un{
        double d;
        char c[8];
    }u;
#ifdef PRINTSTAT
    std::cout<<"-----------------------------"<<std::endl;
#endif
    if(ss.get() != '{') {std::cout<<"DenseLayer missing {"<<std::endl;return false;}
    for( auto & b: *_bias) {
        ss.read(u.c, 8);
        b = (biasType)(round(u.d * _biasScale)); 
#ifdef PRINTSTAT
        if (b == 0) { ++count;}
        //std::cout<<b<<std::endl;
        min = std::min(min, u.d);
        max = std::max(max, u.d);
#endif
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
#ifdef PRINTSTAT
    std::cout<<"b min "<<min<<std::endl;
    std::cout<<"b MAX "<<max<<std::endl;
    std::cout<<"B=0 :#"<<count<<std::endl;
    min = 1e8;
    max = -1e8;
    count = 0;
#endif
    for( size_t idx = 0; idx < (*_weight).size(); ++idx) 
    //for( auto & w: *_weight)
    {
        unsigned int i = idx / _outputSize;
        unsigned int o = idx % _outputSize;
        ss.read(u.c, 8);
        (*_weight)[_calcWeightIndex(i, o)] = (weightType)(round(u.d * _weightScale)); 
#ifdef PRINTSTAT
        if((*_weight)[_calcWeightIndex(i, o)] == 0) { ++count;}
        //std::cout<<w<<std::endl;
        min = std::min(min, u.d);
        max = std::max(max, u.d);
#endif
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
#ifdef PRINTSTAT
    std::cout<<"w min "<<min<<std::endl;
    std::cout<<"w MAX "<<max<<std::endl;
    std::cout<<"w=0 :#"<<count<<std::endl;
#endif
    if(ss.get() != '}') {std::cout<<"DenseLayer missing }"<<std::endl;return false;} 
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}

template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
const std::vector<outType>& DenseLayer<inputType, inputSize, outputSize>::output() const {return _output;}


template class DenseLayer<flOutType, 512, 32>;
template class DenseLayer<outType, 32, 32>;
template class DenseLayer<outType, 32, 1>;