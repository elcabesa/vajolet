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

#include "activation.h"
#include "denseLayer.h"
#include "input.h"

DenseLayer::DenseLayer(const unsigned int inputSize, const unsigned int outputSize, Activation& act, std::vector<double>* bias, std::vector<double>* weight,const double stdDev):
    Layer{inputSize, outputSize, stdDev},
    _bias(bias),
    _weight(weight),
    _act(act)
    
{
    _netOutput.resize(outputSize, 0.0);
}

DenseLayer::~DenseLayer() {}

void DenseLayer::_calcNetOut(const Input& input, bool incremental) {
    assert(input.size() == _inputSize);
    if (!incremental) {
        _netOutput = *_bias;
    }
    unsigned int num = input.getElementNumber();
    //if (incremental)std::cout<<"element number "<< num<<std::endl;
    for (unsigned int idx = 0; idx < num; ++idx) {
        auto& el = input.getElementFromIndex(idx);
        //if (incremental)std::cout<<"element idx "<< idx<<" index "<<el.first<< " value "<<el.second<<std::endl;
        for (unsigned int o = 0; o < _outputSize; ++o) {
            _netOutput[o] += el.second * (*_weight)[_calcWeightIndex(el.first,o)];
        }
    }  
}

void DenseLayer::_calcOut() {
    for(unsigned int o=0; o < _outputSize; ++o) {
        _output.set(o, _act.propagate(_netOutput[o]));
    }
}

void DenseLayer::propagate(const Input& input) {
    _calcNetOut(input);
    _calcOut();
}

void DenseLayer::incrementalPropagate(const Input& input) {
    _calcNetOut(input, true);
    _calcOut();
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
