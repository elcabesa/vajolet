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

#include "featureList.h"
#include "differentialList.h"
#include "parallelDenseLayer.h"

ParallelDenseLayer::ParallelDenseLayer(const unsigned int inputSize, const unsigned int outputSize, std::vector<nnueType>* bias0, std::vector<nnueType>* bias1, std::vector<nnueType>* weight0, std::vector<nnueType>* weight1):
    Layer{2 * inputSize, 2 * outputSize}, 
    _layerOutputSize(outputSize),
    _bias0(bias0),
    _bias1(bias1),
    _weight0(weight0),
    _weight1(weight1),
    _output(2 * outputSize)
{}

ParallelDenseLayer::~ParallelDenseLayer() {
    /*std::cout<<"Pmin "<<_min <<std::endl;
    std::cout<<"PMAX "<<_max <<std::endl;*/
}

unsigned int ParallelDenseLayer::_calcWeightIndex(const unsigned int i, const unsigned int o) const {
    assert(o + i * _layerOutputSize < _weight0->size());
    return o + i * _layerOutputSize;

}

void ParallelDenseLayer::propagate(const FeatureList& l, const FeatureList& h) {
    for (unsigned int o = 0; o < _layerOutputSize; ++o) {
        _output[o] = (*_bias0)[o];
    }

    for (unsigned int o = 0; o < _layerOutputSize; ++o) {
        _output[_layerOutputSize + o ] = (*_bias1)[o];
    }

    for (unsigned int idx = 0; idx < l.size(); ++idx) {
        unsigned int in = l.get(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[o] += (*_weight0)[_calcWeightIndex(in, o)];
        }
    } 

    for (unsigned int idx = 0; idx < h.size(); ++idx) {
        unsigned int in = h.get(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[_layerOutputSize + o] += (*_weight1)[_calcWeightIndex(in, o)];
        }
    } 
    /*std::cout<<"----------------"<<std::endl;
    for (unsigned int o = 0; o < _outputSize; ++o) {
        std::cout<<_output[o] /4096.0 <<std::endl;
    }*/
    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, nnueType(_output[o]));
        _min = std::min(_min, nnueType(_output[o]));
    }*/
}

void ParallelDenseLayer::incrementalPropagate(const DifferentialList& l, const DifferentialList& h) {
    for(unsigned int idx = 0; idx < l.addSize(); ++idx) {
        unsigned int in = l.addList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[o] += (*_weight0)[_calcWeightIndex(in, o)];
        }
    }

    for(unsigned int idx = 0; idx < l.removeSize(); ++idx) {
        unsigned int in = l.removeList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[o] -= (*_weight0)[_calcWeightIndex(in, o)];
        }
    }

    for(unsigned int idx = 0; idx < h.addSize(); ++idx) {
        unsigned int in = h.addList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[_layerOutputSize + o] += (*_weight1)[_calcWeightIndex(in, o)];
        }
    }

    for(unsigned int idx = 0; idx < h.removeSize(); ++idx) {
        unsigned int in = h.removeList(idx);
        for (unsigned int o = 0; o < _layerOutputSize; ++o) {
            _output[_layerOutputSize + o] -= (*_weight1)[_calcWeightIndex(in, o)];
        }
    }

    /*for (unsigned int o = 0; o < _outputSize; ++o) {
        _max = std::max(_max, nnueType(_output[o]));
        _min = std::min(_min, nnueType(_output[o]));
    }*/
}

void ParallelDenseLayer::propagate(const std::vector<nnueType>& ) {
    std::cout<<"ARGHHHHH"<<std::endl;
}

int32_t ParallelDenseLayer::propagate(const std::vector<nnueType>&, const unsigned int, unsigned int) {
    std::cout<<"ARGHHHHH"<<std::endl;
    return 0;
}


bool ParallelDenseLayer::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE PARALLEL DENSE LAYER"<<std::endl;
    if(ss.get() != '{') {std::cout<<"ParallelDenseLayer missing {"<<std::endl;return false;}
    
    if(!_deserialize(ss, _bias0, _weight0)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    if(!_deserialize(ss, _bias1, _weight1)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    
    if(ss.get() != '}') {std::cout<<"ParallelDenseLayer missing }"<<std::endl;return false;}
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}

//#define PRINTSTAT
bool ParallelDenseLayer::_deserialize(std::ifstream& ss, std::vector<nnueType>* bias, std::vector<nnueType>* weight) {
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
        b = (nnueType)(round(u.d * 1024)); //Q10
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
        w = (nnueType)(round(u.d * 1024)); //Q10
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

const std::vector<nnueType>& ParallelDenseLayer::output() const {return _output;}