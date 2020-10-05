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

#include "parallelDenseLayer.h"

ParallelDenseLayer::ParallelDenseLayer(const unsigned int inputSize, const unsigned int outputSize, std::vector<std::vector<double>*> biases, std::vector<std::vector<double>*> weights):
    Layer{2 * inputSize, 2 * outputSize}, _layerInputSize(inputSize), _layerOutputSize(outputSize)
{
    for(unsigned int n = 0 ; n < 2; ++n){
        _parallelLayers.emplace_back(DenseLayer(_layerInputSize, _layerOutputSize, Layer::activationType::linear, biases[n], weights[n]));
    }    
}

ParallelDenseLayer::~ParallelDenseLayer() {}

DenseLayer& ParallelDenseLayer::getLayer(unsigned int i) {
    return _parallelLayers[i];
}

unsigned int ParallelDenseLayer::_calcBiasIndex(const unsigned int layer, const unsigned int offset) const {
    assert(offset < _layerOutputSize);
    unsigned int x = layer * _layerOutputSize + offset;
    assert(x < _outputSize);
    return x;
}

void ParallelDenseLayer::propagate(const FeatureList&) {
    std::cout<<"AHHHHHHHHHHHHHHHHHHH"<<std::endl;
}

void ParallelDenseLayer::propagate(const FeatureList& l, const FeatureList& h) {

    _parallelLayers[0].propagate(l);
    // copy back output
    // TODO remove, write directly to output
    {
        auto& out = _parallelLayers[0].output();
        unsigned int outIndex = 0;
        for(auto el: out) {
            _output[_calcBiasIndex(0, outIndex)] = el; 
            ++outIndex;
        }
    }

    _parallelLayers[1].propagate(h);
    // copy back output
    // TODO remove, write directly to output
    {
        auto& out = _parallelLayers[1].output();
        unsigned int outIndex = 0;
        for(auto el: out) {
            _output[_calcBiasIndex(1, outIndex)] = el; 
            ++outIndex;
        }  
    }
}

void ParallelDenseLayer::incrementalPropagate(const DifferentialList&) {
    std::cout<<"AHHHHHHHHHHHHHHHHHHHh"<<std::endl;
}
void ParallelDenseLayer::incrementalPropagate(const DifferentialList& l, const DifferentialList& h) {
    _parallelLayers[0].incrementalPropagate(l);
    // copy back output
    // TODO remove, write directly to output
    {
        auto& out = _parallelLayers[0].output();
        unsigned int outIndex = 0;
        for(auto el: out) {
            _output[_calcBiasIndex(0, outIndex)] = el; 
            ++outIndex;
        }
    }

    _parallelLayers[1].incrementalPropagate(h);
    // copy back output
    // TODO remove, write directly to output
    {
        auto& out = _parallelLayers[1].output();
        unsigned int outIndex = 0;
        for(auto el: out) {
            _output[_calcBiasIndex(1, outIndex)] = el; 
            ++outIndex;
        }  
    }
}

void ParallelDenseLayer::propagate(const std::vector<double>& ) {
    std::cout<<"ARGHHHHH"<<std::endl;
}

void ParallelDenseLayer::incrementalPropagate(const std::vector<double>& ) {
    std::cout<<"ARGHHHHH2"<<std::endl;
}

bool ParallelDenseLayer::deserialize(std::ifstream& ss) {
    //std::cout<<"DESERIALIZE PARALLEL DENSE LAYER"<<std::endl;
    if(ss.get() != '{') {std::cout<<"ParallelDenseLayer missing {"<<std::endl;return false;}
    
    if(!_parallelLayers[0].deserialize(ss)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    if(!_parallelLayers[1].deserialize(ss)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    
    if(ss.get() != '}') {std::cout<<"ParallelDenseLayer missing }"<<std::endl;return false;}
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
