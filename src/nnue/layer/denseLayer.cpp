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
    _act(act),
    _output(outputSize)
{}

DenseLayer::~DenseLayer() {
    //std::cout<<"min "<<_min <<std::endl;
    //std::cout<<"MAX "<<_max <<std::endl;
}

void DenseLayer::propagate(const FeatureList&, const FeatureList&) {
    std::cout<<"AHHHHHHHHHHHHHHHHHHHH"<<std::endl;
}

void DenseLayer::incrementalPropagate(const DifferentialList&, const DifferentialList&) {
    std::cout<<"AAAAAAAAAAAAAAHHH"<<std::endl;
}

nnueType DenseLayer::_propagate(const std::vector<nnueType>& input, const unsigned int index) {   
    int32_t out = 0;
    for(unsigned int i = 0; i< _inputSize; ++i) {
        out += input[i] * (*_weight)[index + i]; // Q12 * Q15
    }
    return out >>15;
}

void DenseLayer::propagate(const std::vector<nnueType>& input) {

    unsigned int index = 0;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        
        _output[o] = (*_bias)[o]; // Q12
        
        _output[o] += _propagate(input, index); //Q12

        if(_act == activationType::relu) {
            _output[o] = std::max(_output[o], (nnueType)(0.0)); 
        }
        index += _inputSize;
    }

    std::cout<<"----------------"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        std::cout<<_output[o] /4096.0 <<std::endl;
    }

    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, nnueType(_output[o]));
        _min = std::min(_min, nnueType(_output[o]));
    }*/
}

unsigned int DenseLayer::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _outputSize < _weight->size());
    return o * _inputSize + i;

}

bool DenseLayer::deserialize(std::ifstream& ss) {
    /*nnueType min = 1e8;
    nnueType max = -1e8;*/
    //std::cout<<"DESERIALIZE DENSE LAYER"<<std::endl;
    union un{
        double d;
        char c[8];
    }u;
    //std::cout<<"-----------------------------"<<std::endl;
    if(ss.get() != '{') {std::cout<<"DenseLayer missing {"<<std::endl;return false;}
    for( auto & b: *_bias) {
        ss.read(u.c, 8);
        b = (nnueType)(u.d * 4096);
        //std::cout<<b<<std::endl;
        //min = std::min(min, nnueType(b));
        //max = std::max(max, nnueType(b));
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    /*std::cout<<"b min "<<min<<std::endl;
    std::cout<<"b MAX "<<max<<std::endl;
    min = 1e8;
    max = -1e8;*/
    for( size_t idx = 0; idx < (*_weight).size(); ++idx) 
    //for( auto & w: *_weight)
    {
        unsigned int i = idx / _outputSize;
        unsigned int o = idx % _outputSize;
        ss.read(u.c, 8);
        (*_weight)[_calcWeightIndex(i, o)] = (nnueType)(u.d * 32768);
        //std::cout<<w<<std::endl;
        //min = std::min(min, nnueType(w));
        //max = std::max(max, nnueType(w));
        if(ss.get() != ',') {std::cout<<"DenseLayer missing ,"<<std::endl;return false;} 
        if(ss.get() != ' ') {std::cout<<"DenseLayer missing space"<<std::endl;return false;}
    }
    //std::cout<<"w min "<<min<<std::endl;
    //std::cout<<"w MAX "<<max<<std::endl;
    if(ss.get() != '}') {std::cout<<"DenseLayer missing }"<<std::endl;return false;} 
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}

const std::vector<nnueType>& DenseLayer::output() const {return _output;}