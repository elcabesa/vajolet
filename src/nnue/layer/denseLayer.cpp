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
#include "input.h"

DenseLayer::DenseLayer(const unsigned int inputSize, const unsigned int outputSize, activationType act, std::vector<double>* bias, std::vector<double>* weight):
    Layer{inputSize, outputSize},
    _bias(bias),
    _weight(weight),
    _act(act)  
{}

DenseLayer::~DenseLayer() {}

void DenseLayer::_calcNetOut(const FeatureList& input) {
    //assert(input.size() == _inputSize);
    
    _output = *_bias;

    for (unsigned int idx = 0; idx < input.size(); ++idx) {
        unsigned int in = input.get(idx);
        for (unsigned int o = 0; o < _outputSize; ++o) {
            _output[o] += (*_weight)[_calcWeightIndex(in, o)];
        }
    }  
}

void DenseLayer::_calcNetOutIncremental(const DifferentialList& input) {
    //assert(input.size() == _inputSize);

    for(unsigned int idx = 0; idx < input.addSize(); ++idx) {
        unsigned int in = input.addList(idx);
        for (unsigned int o = 0; o < _outputSize; ++o) {
            _output[o] += (*_weight)[_calcWeightIndex(in, o)];
        }
    }

    for(unsigned int idx = 0; idx < input.removeSize(); ++idx) {
        unsigned int in = input.removeList(idx);
        for (unsigned int o = 0; o < _outputSize; ++o) {
            _output[o] -= (*_weight)[_calcWeightIndex(in, o)];
        }
    }
}


void DenseLayer::_calcNetOut2(const std::vector<double>& input, bool incremental) {
    assert(input.size() == _inputSize);
    if (!incremental) {
        _output = *_bias;
    }
    unsigned int index = 0;
    for(double value: input) {
        for (unsigned int o = 0; o < _outputSize; ++o) {
            _output[o] += value * (*_weight)[_calcWeightIndex(index, o)];
        }
        ++index;
    }
}

void DenseLayer::_calcOut() {
    if(_act == activationType::relu) {
        for(unsigned int o=0; o < _outputSize; ++o) {
            _output[o] = std::max(_output[o], 0.0); 
        }
    }
}

void DenseLayer::propagate(const FeatureList& input) {
    _calcNetOut(input);
}
void DenseLayer::propagate(const FeatureList&, const FeatureList&) {
    std::cout<<"AHHHHHHHHHHHHHHHHHHHH"<<std::endl;
}

void DenseLayer::incrementalPropagate(const DifferentialList& input) {
    _calcNetOutIncremental(input);
    _calcOut();
}

void DenseLayer::incrementalPropagate(const DifferentialList&, const DifferentialList&) {
    std::cout<<"AAAAAAAAAAAAAAHHH"<<std::endl;
}

void DenseLayer::propagate(const std::vector<double>& input) {
    _calcNetOut2(input);
    _calcOut();
}

void DenseLayer::incrementalPropagate(const std::vector<double>&) {
    std::cout<<"AHHHHHHHHHHHHH"<<std::endl;
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
        b = u.d;
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    for( auto & w: *_weight) {
        ss.read(u.c, 8);
        w = u.d;
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '}') {std::cout<<"DenseLayer missing }"<<std::endl;return false;} 
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
