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
#include <algorithm>

#include "denseLayer.h"
#include "differentialList.h"
#include "featureList.h"


#define FASTER_CODE

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
double DenseLayer<inputType, accType, inputSize, outputSize>::_min = 1e8;

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
double DenseLayer<inputType, accType, inputSize, outputSize>::_max = -1e8;

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
bool DenseLayer<inputType, accType, inputSize, outputSize>::_overflow = false;

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
std::vector<bool> DenseLayer<inputType, accType, inputSize, outputSize>::_deadAccumulator;

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
DenseLayer<inputType, accType, inputSize, outputSize>::DenseLayer(std::vector<biasType>* bias, std::vector<weightType>* weight, outType scaleIn, outType scaleOut, unsigned int outputBucket):
    _inputSize(inputSize),
    _outputSize(outputSize),
    _outputBucket(outputBucket),
    _bias(bias),
    _weight(weight),
    _output(outputSize),
    _outputScRelu(outputSize),
    _scaleIn(scaleIn),
    _scaleOut(scaleOut)
{
#ifdef CALC_DEBUG_DATA
    _deadAccumulator.resize(outputSize, true);
    for (std::size_t t{}; t != _deadAccumulator.size(); ++t) {
        std::cout << (t ? ", " : "") << _deadAccumulator[t];
    }
    std::cout<<std::endl;
#endif

}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
DenseLayer<inputType, accType, inputSize, outputSize>::~DenseLayer() {
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
accType DenseLayer<inputType, accType, inputSize, outputSize>::propagateOut(const std::vector<inputType>& input1, const std::vector<inputType>& input2, const unsigned int bucket) {
    accType out = (*_bias)[_calcBiasIndex(0,bucket)]* _scaleIn; //Q12*Q12

#ifdef FASTER_CODE
    auto* ref = &(_weight->data()[_calcWeightIndex(0, 0, bucket)]);
    auto *in = input1.data();
#endif
    for(unsigned int i = 0; i < _inputSize / 2; ++i) {
#ifndef FASTER_CODE
        out += input1[i] * (*_weight)[_calcWeightIndex(i, 0, bucket)];
#else
#ifdef CALC_DEBUG_DATA
        if((*in * *ref) > 0 && std::numeric_limits<accType>::max() - (*in * *ref) < out) {_overflow = true; std::cout<<"overflow1"<<std::endl;exit(1);}
        if((*in * *ref) < 0 && std::numeric_limits<accType>::min() - (*in * *ref) > out) {_overflow = true; std::cout<<"overflow2"<<std::endl;exit(1);}
#endif
        out += *in * *ref;
        //std::cout<< out <<"["<<i<< "] " <<*in <<" "<< *ref<<std::endl;
        ++in;
        ++ref;
#endif
#ifdef CALC_DEBUG_DATA
        if(out > _max ) _max = out;
        if(out < _min ) _min = out;
#endif
    }

#ifdef FASTER_CODE
    in = input2.data();
#endif
    for(unsigned int i = 0; i < _inputSize / 2; ++i) {
#ifndef FASTER_CODE
        out += input2[i] * (*_weight)[_calcWeightIndex(i + _inputSize / 2, 0, bucket)];
#else
#ifdef CALC_DEBUG_DATA
        if((*in * *ref) > 0 && std::numeric_limits<accType>::max() - (*in * *ref) < out) {_overflow = true; std::cout<<"overflow3"<<std::endl;exit(1);}
        if((*in * *ref) < 0 && std::numeric_limits<accType>::min() - (*in * *ref) > out) {_overflow = true; std::cout<<"overflow4"<<std::endl;exit(1);}
#endif
        out += *in * *ref;
        //std::cout<< out <<" " <<*in <<" "<< *ref<<std::endl;
        ++in;
        ++ref;
#endif
#ifdef CALC_DEBUG_DATA
        if(out > _max ) _max = out;
        if(out < _min ) _min = out;
#endif
    }

#ifdef CALC_DEBUG_DATA
    _deadAccumulator[0] = false;
#endif
    //std::cout<<"ACCOUT "<<out- (*_bias)[0]* scaleFL<<std::endl;
    return out;
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
void DenseLayer<inputType, accType, inputSize, outputSize>::propagate(const FeatureList& l) {
    //std::cout<<"biases"<<std::endl;
    auto out = _output.data();
    auto outScRelu = _outputScRelu.data();
    auto bias = _bias->data();
    for (unsigned int o = 0; o < _outputSize; ++o) {
        out[o] = bias[o];
        //std::cout<<"out "<<out[o]<<std::endl;
    }

    //std::cout<<"accumulate"<<std::endl;
    for (unsigned int idx = 0; idx < l.size(); ++idx) {
        unsigned int in = l.get(idx);
#ifdef FASTER_CODE
        auto offset = _calcWeightIndex(in, 0, 0);
        auto* ref = &(_weight->data()[offset]);
#endif
        for (unsigned int o = 0; o < _outputSize; ++o) {
#ifndef FASTER_CODE
            out[o] += (*_weight)[_calcWeightIndex(in, o, 0)];
#else
#ifdef CALC_DEBUG_DATA
            if((*ref) > 0 && std::numeric_limits<accType>::max() - *ref < out[o]) {_overflow = true; std::cout<<"overflow5"<<std::endl;exit(1);}
            if((*ref) < 0 && std::numeric_limits<accType>::min() - *ref > out[o]) {_overflow = true; std::cout<<"overflow6"<<std::endl;exit(1);}
#endif
            out[o] += *ref;
            ++ref;
#endif
#ifdef CALC_DEBUG_DATA
            if(out[o] > _max ) _max = out[o];
            if(out[o] < _min ) _min = out[o];
#endif
        }
    }

    for (unsigned int o = 0; o < _outputSize; ++o) {
        //std::cout<<"acc "<<out[o]<<std::endl;
        int32_t t = std::clamp(out[o], (accType)(0), (accType)(scaleFL));  //Q12;
        t = t * t;
        outScRelu[o] = t;

#ifdef CALC_DEBUG_DATA
        if(outScRelu[o] > 0 && outScRelu[o] < scaleFL * scaleFL) {
            _deadAccumulator[o] = false;
            //std::cout<<"ALIVE"<<std::endl;
        }
#endif
    }
}


template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
void DenseLayer<inputType, accType, inputSize, outputSize>::incrementalPropagate(const DifferentialList& l) {

    auto out = _output.data();
    auto outScRelu = _outputScRelu.data();

    for(unsigned int idx = 0; idx < l.addSize(); ++idx) {
        unsigned int in = l.addList(idx);

#ifdef FASTER_CODE
        auto offset = _calcWeightIndex(in, 0, 0);
        auto* ref = &(_weight->data()[offset]);
#endif

        for (unsigned int o = 0; o < _outputSize; ++o) {
#ifndef FASTER_CODE
            out[o] += (*_weight)[_calcWeightIndex(in, o, 0)];
#else
#ifdef CALC_DEBUG_DATA
            if((*ref) > 0 && std::numeric_limits<accType>::max() - *ref < out[o]) {_overflow = true; std::cout<<"overflow7"<<std::endl;exit(1);}
            if((*ref) < 0 && std::numeric_limits<accType>::min() - *ref > out[o]) {_overflow = true; std::cout<<"overflow8"<<std::endl;exit(1);}
#endif
            out[o] += *ref;
            ++ref;
#endif
#ifdef CALC_DEBUG_DATA
            if(out[o] > _max ) _max = out[o];
            if(out[o] < _min ) _min = out[o];
#endif
        }
    }

    for(unsigned int idx = 0; idx < l.removeSize(); ++idx) {
        unsigned int in = l.removeList(idx);
#ifdef FASTER_CODE
        auto offset = _calcWeightIndex(in, 0, 0);
        auto* ref = &(_weight->data()[offset]);
#endif
        for (unsigned int o = 0; o < _outputSize; ++o) {
#ifndef FASTER_CODE
            out[o] -= (*_weight)[_calcWeightIndex(in, o, 0)];
#else
#ifdef CALC_DEBUG_DATA
            if((*ref) < 0 && std::numeric_limits<accType>::max() + *ref < out[o]) {_overflow = true; std::cout<<"overflow1"<<std::endl;exit(1);}
            if((*ref) > 0 && std::numeric_limits<accType>::min() + *ref > out[o]) {_overflow = true; std::cout<<"overflow1"<<std::endl;exit(1);}
#endif
            out[o] -= *ref;
            ++ref;
#endif
#ifdef CALC_DEBUG_DATA
            if(out[o] > _max ) _max = out[o];
            if(out[o] < _min ) _min = out[o];
#endif
        }
    }

    for (unsigned int o = 0; o < _outputSize; ++o) {
        int32_t t = std::clamp(out[o], (accType)(0), (accType)(scaleFL));  //Q12;
        t = t * t;
        outScRelu[o] = t;
#ifdef CALC_DEBUG_DATA
        if(outScRelu[o] > 0 && outScRelu[o] < scaleFL * scaleFL) {
            _deadAccumulator[o] = false;
            //std::cout<<"ALIVE"<<std::endl;
        }
#endif
    }
}
template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
void DenseLayer<inputType, accType, inputSize, outputSize>::printStat() const {
#ifdef CALC_DEBUG_DATA
    std::cout<<"min:"<<_min<<" max:"<<_max<<" overflow:"<<_overflow<<std::endl;
    int count = 0 ;
    for (std::size_t t{}; t != _deadAccumulator.size(); ++t) {
        if(_deadAccumulator[t]) ++count;
        //std::cout << (t ? ", " : "") << _deadAccumulator[t];
    }

    std::cout<<count<<std::endl;
#endif
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
unsigned int DenseLayer<inputType, accType, inputSize, outputSize>::_calcWeightIndex(const unsigned int i, const unsigned int o, const unsigned int outputBucket) const {
    assert(o + i * _outputSize + outputBucket * _inputSize * _outputSize < _weight->size());
    return o + i * _outputSize + outputBucket * _inputSize * _outputSize;
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
unsigned int DenseLayer<inputType, accType, inputSize, outputSize>::_calcBiasIndex(const unsigned int o, const unsigned int outputBucket) const {
    assert(o + outputBucket *  _outputSize < _bias->size());
    return o + outputBucket *  _outputSize;
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
bool DenseLayer<inputType, accType, inputSize, outputSize>::deserialize(std::istream& ss) {
#ifdef PRINTSTAT
    unsigned int totcount = 0;
    unsigned int count = 0;
    double min = 1e8;
    double max = -1e8;
#endif
    union _bb{
        int16_t d;
        char c[2];
    }bb;

    union _ww{
        int16_t d;
        char c[2];
    }ww;

#ifdef PRINTSTAT
    std::cout<<"-----------------------------"<<std::endl;
#endif
    for (size_t i = 0; i < _inputSize && ss.good(); ++i) {
        for (size_t b = 0; b < _outputBucket; ++b) {
            for (size_t o = 0; o < _outputSize && ss.good(); ++o) {
                 ss.read(ww.c, 2);
                 (*_weight)[_calcWeightIndex(i, o, b)] = (weightType)(std::round(ww.d)); // Q12
                 std::cout<<"w i "<<i<<" o "<<o<<" b "<<b<<" idx "<<_calcBiasIndex(o, b)<<" "<<(*_weight)[_calcWeightIndex(i, o, b)]<<std::endl;
#ifdef PRINTSTAT
                 ++totcount;
                 if((*_weight)[_calcWeightIndex(i, o, b)] == 0) { ++count;}
                 min = std::min(min,  double(ww.d));
                 max = std::max(max,  double(ww.d));
#endif
            }
        }
    }
#ifdef PRINTSTAT
    std::cout<<"w min "<<min<<std::endl;
    std::cout<<"w MAX "<<max<<std::endl;
    std::cout<<"w=0 :#"<<count<<std::endl;
    std::cout<<"tot :#"<<totcount<<std::endl;
    min = 1e8;
    max = -1e8;
    count = 0;
    totcount = 0;
#endif

    for (size_t b = 0; b < _outputBucket; ++b) {
        for (size_t o = 0; o < _outputSize && ss.good(); ++o) {
            ss.read(bb.c, 2);
            (*_bias)[_calcBiasIndex(o, b)] = (biasType)(std::round(bb.d)); // Q12
            std::cout<<"b o "<<o<<" b "<<b<<" idx "<<_calcBiasIndex(o, b)<<" "<<(biasType)(std::round(bb.d))<<std::endl;
#ifdef PRINTSTAT
            ++totcount;
            if ( (*_bias)[_calcBiasIndex(o, b)] == 0) { ++count;}
            min = std::min(min, double(bb.d));
            max = std::max(max,  double(bb.d));
#endif
        }
    }
#ifdef PRINTSTAT
    std::cout<<"b min "<<min<<std::endl;
    std::cout<<"b MAX "<<max<<std::endl;
    std::cout<<"B=0 :#"<<count<<std::endl;
    std::cout<<"tot :#"<<totcount<<std::endl;
#endif
    return true;
}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
const std::vector<accType>& DenseLayer<inputType, accType, inputSize, outputSize>::output() const {return _output;}

template <typename inputType, typename accType, unsigned int inputSize, unsigned int outputSize>
const std::vector<outType>& DenseLayer<inputType, accType, inputSize, outputSize>::outputScRelu() const {return _outputScRelu;}


template class DenseLayer<outType, accumulatorTypeFL, inputSize, accumulatorSize>;
template class DenseLayer<outType, accumulatorTypeOut, accumulatorSize * 2, outSize>;
