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

#include "denseLayer.h"
#include "differentialList.h"
#include "featureList.h"

DenseLayer::DenseLayer(const unsigned int inputSize, const unsigned int outputSize, activationType act, std::vector<nnueType>* bias, std::vector<nnueType>* weight):
    Layer{inputSize, outputSize},
    _bias(bias),
    _weight(weight),
    _act(act)  
{}

DenseLayer::~DenseLayer() {}

void DenseLayer::propagate(const FeatureList&, const FeatureList&) {
    std::cout<<"AHHHHHHHHHHHHHHHHHHHH"<<std::endl;
}

void DenseLayer::incrementalPropagate(const DifferentialList&, const DifferentialList&) {
    std::cout<<"AAAAAAAAAAAAAAHHH"<<std::endl;
}

void DenseLayer::propagate(const std::vector<nnueType>& input) {
    _output = *_bias;

    unsigned int index = 0;
    for(nnueType value: input) {
        for (unsigned int o = 0; o < _outputSize; ++o) {
            _output[o] += value * (*_weight)[_calcWeightIndex(index, o)];
        }
        ++index;
    }
    if(_act == activationType::relu) {
        for(unsigned int o = 0; o < _outputSize; ++o) {
            _output[o] = std::max(_output[o], (nnueType)(0.0)); 
        }
    }
}

unsigned int DenseLayer::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _outputSize < _weight->size());
    return o + i * _outputSize;

}

bool DenseLayer::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE DENSE LAYER"<<std::endl;
    union un{
        double d;
        char c[8];
    }u;
    if(ss.get() != '{') {std::cout<<"DenseLayer missing {"<<std::endl;return false;}
    for( auto & b: *_bias) {
        ss.read(u.c, 8);
        b = (nnueType)(u.d);
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    for( auto & w: *_weight) {
        ss.read(u.c, 8);
        w = (nnueType)(u.d);
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '}') {std::cout<<"DenseLayer missing }"<<std::endl;return false;} 
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
