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

int32_t DenseLayer::propagate(const std::vector<nnueType>& input, const unsigned int index, unsigned int o) {   
    int32_t out = (*_bias)[o]; // Q24;
    for(unsigned int i = 0; i< _inputSize; ++i) {
        out += input[i] * (*_weight)[index + i]; // Q10 * Q14
    }
    //if( std::abs(out >>14)>32767 ) {std::cout<<"WARNING: "<<std::abs(out >>14)/1024.0<<std::endl;}
    return out; //Q24
}

void DenseLayer::propagate(const std::vector<nnueType>& input) {

    unsigned int index = 0;
    for (unsigned int o = 0; o < _outputSize; ++o) {        
       int32_t out = propagate(input, index, o); //Q24

        if(_act == activationType::relu) {
            out = std::max(out, 0); // Q24
        }
        _output[o] = out >>14; // Q10
        index += _inputSize;
    }

    /*std::cout<<"----------------"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        std::cout<<_output[o] /1024.0 <<std::endl;
    }*/

    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, nnueType(_output[o]));
        _min = std::min(_min, nnueType(_output[o]));
    }*/
}

unsigned int DenseLayer::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _outputSize < _weight->size());
    return o * _inputSize + i;

}

//#define PRINTSTAT
bool DenseLayer::deserialize(std::ifstream& ss) {
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
        b = (nnueType)(round(u.d * 16777216)); //Q24
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
        (*_weight)[_calcWeightIndex(i, o)] = (nnueType)(round(u.d * 16384)); //Q14
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

const std::vector<nnueType>& DenseLayer::output() const {return _output;}