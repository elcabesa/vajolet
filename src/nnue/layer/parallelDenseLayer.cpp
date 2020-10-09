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

#include "featureList.h"
#include "differentialList.h"
#include "parallelDenseLayer.h"

template <unsigned int inputSize, unsigned int outputSize> 
ParallelDenseLayer<inputSize, outputSize>::ParallelDenseLayer(std::vector<flBiasType>* bias0, std::vector<flBiasType>* bias1, std::vector<flWeightType>* weight0, std::vector<flWeightType>* weight1, unsigned int biasScale, unsigned int weightScale, unsigned int outShift):
    _inputSize(2 * inputSize),
    _outputSize(2 * outputSize),
    _biasScale(biasScale),
    _weightScale(weightScale),
    _outShift(outShift),
    _layerOutputSize(outputSize),
    _bias0(bias0),
    _bias1(bias1),
    _weight0(weight0),
    _weight1(weight1),
    _output(2 * outputSize)
{}

template <unsigned int inputSize, unsigned int outputSize> 
ParallelDenseLayer<inputSize, outputSize>::~ParallelDenseLayer() {
    /*std::cout<<"Pmin "<<_min <<std::endl;
    std::cout<<"PMAX "<<_max <<std::endl;*/
}

template <unsigned int inputSize, unsigned int outputSize> 
unsigned int ParallelDenseLayer<inputSize, outputSize>::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _layerOutputSize < _weight0->size());
    return o + i * _layerOutputSize;

}

//#define _WARNING (100)
template <unsigned int inputSize, unsigned int outputSize> 
void ParallelDenseLayer<inputSize, outputSize>::propagate(const FeatureList& l, const FeatureList& h) {
    for (unsigned int o = 0; o < _layerOutputSize; ++o) {
        _output[o] = (*_bias0)[o];
        //if(std::abs(_output[o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
    }

    for (unsigned int o = 0; o < _layerOutputSize; ++o) {
        _output[_layerOutputSize + o ] = (*_bias1)[o];
        //if(std::abs(_output[_layerOutputSize + o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
    }

    for (unsigned int idx = 0; idx < l.size(); ++idx) {
        unsigned int in = l.get(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[o] += (*_weight0)[_calcWeightIndex(in, o)];
            //if(std::abs(_output[o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
        }
    } 

    for (unsigned int idx = 0; idx < h.size(); ++idx) {
        unsigned int in = h.get(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[_layerOutputSize + o] += (*_weight1)[_calcWeightIndex(in, o)];
            //if(std::abs(_output[_layerOutputSize + o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
        }
    }

    for (unsigned int o = 0; o < _outputSize; ++o) {        
        _output[o] = std::min(std::max(_output[o], flOutType(0)), flOutType(127));
    }
    /*std::cout<<"----------------"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        std::cout<<_accumulator[o] /1024.0 <<std::endl;
    }*/
    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, double(_accumulator[o]));
        _min = std::min(_min, double(_accumulator[o]));
    }*/
}

template <unsigned int inputSize, unsigned int outputSize> 
void ParallelDenseLayer<inputSize, outputSize>::incrementalPropagate(const DifferentialList& l, const DifferentialList& h) {
    for(unsigned int idx = 0; idx < l.addSize(); ++idx) {
        unsigned int in = l.addList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[o] += (*_weight0)[_calcWeightIndex(in, o)];
            //if(std::abs(_output[o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
        }
    }

    for(unsigned int idx = 0; idx < l.removeSize(); ++idx) {
        unsigned int in = l.removeList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[o] -= (*_weight0)[_calcWeightIndex(in, o)];
            //if(std::abs(_output[o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
        }
    }

    for(unsigned int idx = 0; idx < h.addSize(); ++idx) {
        unsigned int in = h.addList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[_layerOutputSize + o] += (*_weight1)[_calcWeightIndex(in, o)];
            //if(std::abs(_output[o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
        }
    }

    for(unsigned int idx = 0; idx < h.removeSize(); ++idx) {
        unsigned int in = h.removeList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[_layerOutputSize + o] -= (*_weight1)[_calcWeightIndex(in, o)];
            //if(std::abs(_output[o])>_WARNING) {std::cout<<"WARNING"<<std::endl;}
        }
    }

    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, double(_output[o]));
        _min = std::min(_min, double(_output[o]));
    }*/
}

template <unsigned int inputSize, unsigned int outputSize> 
bool ParallelDenseLayer<inputSize, outputSize>::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE PARALLEL DENSE LAYER"<<std::endl;
    if(ss.get() != '{') {std::cout<<"ParallelDenseLayer missing {"<<std::endl;return false;}
    
    if(!_deserialize(ss, _bias0, _weight0)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    if(!_deserialize(ss, _bias1, _weight1)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    
    if(ss.get() != '}') {std::cout<<"ParallelDenseLayer missing }"<<std::endl;return false;}
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}

//#define PRINTSTAT
template <unsigned int inputSize, unsigned int outputSize> 
bool ParallelDenseLayer<inputSize, outputSize>::_deserialize(std::ifstream& ss, std::vector<flBiasType>* bias, std::vector<flWeightType>* weight) {
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
    for( auto & b: *bias) {
        ss.read(u.c, 8);
        b = (flBiasType)(round(u.d * _biasScale));
#ifdef PRINTSTAT
        if (b == 0) { ++count;}
        //std::cout<<b<<std::endl;
        min = std::min(min, u.d);
        max = std::max(max, u.d);
#endif
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
#ifdef PRINTSTAT
    std::cout<<"b min "<<min<<std::endl;
    std::cout<<"b MAX "<<max<<std::endl;
    std::cout<<"B=0 :#"<<count<<std::endl;
    min = 1e8;
    max = -1e8;
    count = 0;
#endif
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    for( auto & w: *weight) {
        ss.read(u.c, 8);
        w = (flWeightType)(round(u.d * _weightScale));
#ifdef PRINTSTAT
        if(w == 0) { ++count;}
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

template <unsigned int inputSize, unsigned int outputSize> 
const std::vector<flOutType>& ParallelDenseLayer<inputSize, outputSize>::output() const {return _output;}

template class ParallelDenseLayer<40960, 256>;