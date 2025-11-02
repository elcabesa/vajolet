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


#define FASTER_CODE

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
DenseLayer<inputType, accType, inputSize, outputSize>::DenseLayer(std::vector<biasType>* bias, std::vector<weightType>* weight, outType scale):
    _inputSize(inputSize),
    _outputSize(outputSize),
    _bias(bias),
    _weight(weight),
    _output(outputSize),
    _outputRelu(outputSize),
    _scale(scale)
{}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
DenseLayer<inputType, accType, inputSize, outputSize>::~DenseLayer() {
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
accType DenseLayer<inputType, accType, inputSize, outputSize>::propagateOut(const std::vector<inputType>& input, const unsigned int index, unsigned int o) {
    accType out = (*_bias)[o] * _scale; //Q12*Q12
#ifdef FASTER_CODE
    auto* ref = &(_weight->data()[index]);
    auto *in = input.data();
#endif
    for(unsigned int i = 0; i < _inputSize; ++i) {
#ifndef FASTER_CODE
        out += input[i] * (*_weight)[index + i]; // SBAGLIATO funziona solo per l'uscita' Q12*Q12 = Q24
#else
        out += *in * *ref;
        ++in;
        ++ref;
#endif
    }
    return out;
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
void DenseLayer<inputType, accType, inputSize, outputSize>::propagate(const FeatureList& l) {
    //std::cout<<"biases"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        _output[o] = (*_bias)[o];
    }

    //std::cout<<"accumulate"<<std::endl;
    for (unsigned int idx = 0; idx < l.size(); ++idx) {
        unsigned int in = l.get(idx);
#ifdef FASTER_CODE
        auto offset = _calcWeightIndex(in, 0);
        auto* ref = &(_weight->data()[offset]);
#endif
        for (unsigned int o = 0; o < _outputSize; ++o) {
#ifndef FASTER_CODE
            _output[o] += (*_weight)[_calcWeightIndex(in, o)];
#else
            _output[o] += *ref;
            ++ref;
#endif
        }
    }

    for (unsigned int o = 0; o < _outputSize; ++o) {
        _outputRelu[o] = std::max(_output[o], (accType)(0.0f));  //Q12
    }
}


template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
void DenseLayer<inputType, accType, inputSize, outputSize>::incrementalPropagate(const DifferentialList& l) {
    for(unsigned int idx = 0; idx < l.addSize(); ++idx) {
        unsigned int in = l.addList(idx);

#ifdef FASTER_CODE
        auto offset = _calcWeightIndex(in, 0);
        auto* ref = &(_weight->data()[offset]);
#endif

        for (unsigned int o = 0; o < _outputSize; ++o) {
#ifndef FASTER_CODE
            _output[o] += (*_weight)[_calcWeightIndex(in, o)];
#else
            _output[o] += *ref;
            ++ref;
#endif
        }
    }

    for(unsigned int idx = 0; idx < l.removeSize(); ++idx) {
        unsigned int in = l.removeList(idx);
#ifdef FASTER_CODE
        auto offset = _calcWeightIndex(in, 0);
        auto* ref = &(_weight->data()[offset]);
#endif
        for (unsigned int o = 0; o < _outputSize; ++o) {
#ifndef FASTER_CODE
            _output[o] -= (*_weight)[_calcWeightIndex(in, o)];
#else
            _output[o] -= *ref;
            ++ref;
#endif
        }
    }

    for (unsigned int o = 0; o < _outputSize; ++o) {
        _outputRelu[o] = std::max(_output[o], (accType)(0.0f)); //Q12
    }
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
unsigned int DenseLayer<inputType, accType, inputSize, outputSize>::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _outputSize < _weight->size());
    return o + i * _outputSize;

}

//#define PRINTSTAT
template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
bool DenseLayer<inputType, accType, inputSize, outputSize>::deserialize(std::istream& ss) {
#ifdef PRINTSTAT
    unsigned int count = 0;
    double min = 1e8;
    double max = -1e8;
#endif
    union _bb{
        float d;
        char c[4];
    }bb;

    union _ww{
        float d;
        char c[4];
    }ww;

#ifdef PRINTSTAT
    std::cout<<"-----------------------------"<<std::endl;
#endif
    for( auto & b: *_bias) {
        ss.read(bb.c, 4);
        b = (biasType)(std::round(bb.d * _scale)); // Q12
#ifdef PRINTSTAT
        if (b == 0) { ++count;}
        min = std::min(min, double(b));
        max = std::max(max,  double(b));
#endif
    }
#ifdef PRINTSTAT
    std::cout<<"b min "<<min<<std::endl;
    std::cout<<"b MAX "<<max<<std::endl;
    std::cout<<"B=0 :#"<<count<<std::endl;
    min = 1e8;
    max = -1e8;
    count = 0;
#endif
    for( size_t idx = 0; idx < (*_weight).size(); ++idx) 
    {
        unsigned int i = idx / _outputSize;
        unsigned int o = idx % _outputSize;
        ss.read(ww.c, 4);
        (*_weight)[_calcWeightIndex(i, o)] = (weightType)(std::round(ww.d * _scale)); // Q12
#ifdef PRINTSTAT
        if((*_weight)[_calcWeightIndex(i, o)] == 0) { ++count;}
        min = std::min(min,  double((*_weight)[_calcWeightIndex(i, o)]));
        max = std::max(max,  double((*_weight)[_calcWeightIndex(i, o)]));
#endif
    }
#ifdef PRINTSTAT
    std::cout<<"w min "<<min<<std::endl;
    std::cout<<"w MAX "<<max<<std::endl;
    std::cout<<"w=0 :#"<<count<<std::endl;
#endif
    return true;
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
const std::vector<accType>& DenseLayer<inputType, accType, inputSize, outputSize>::output() const {return _output;}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
const std::vector<outType>& DenseLayer<inputType, accType, inputSize, outputSize>::outputRelu() const {return _outputRelu;}


template class DenseLayer<outType, accumulatorTypeFL, 768, 512>;
template class DenseLayer<outType, accumulatorTypeOut, 512, 1>;
