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
#include "parallelSparse.h"

ParallelDenseLayer::ParallelDenseLayer(const unsigned int number, const unsigned int inputSize, const unsigned int outputSize, std::vector<std::vector<double>*> biases, std::vector<std::vector<double>*> weights):
    Layer{number * inputSize, number * outputSize}, _layerInputSize(inputSize), _layerOutputSize(outputSize)
{
    for(unsigned int n = 0 ; n < number; ++n){
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

void ParallelDenseLayer::propagate(const Input& input) {
    unsigned int LayerNum = 0;
    //_output.clear();
    for(auto& l: _parallelLayers) {
        
        const ParalledSparseInput psi(input, LayerNum, _layerInputSize);
        l.propagate(psi);
        
        // copy back output
        // TODO remove, write directly to output
        auto& out = l.output();
        unsigned int outIndex = 0;
        for(auto el: out) {
            _output[_calcBiasIndex(LayerNum, outIndex)] = el; 
            ++outIndex;
        }
        ++LayerNum;
    }    
}

void ParallelDenseLayer::incrementalPropagate(const Input& input) {
    unsigned int LayerNum = 0;
    //_output.clear();
    for(auto& l: _parallelLayers) {
        
        const ParalledSparseInput psi(input, LayerNum, _layerInputSize);
        l.incrementalPropagate(psi);
        
        // copy back output
        // TODO remove, write directly to output
        auto& out = l.output();
        unsigned int outIndex = 0;
        for(auto el: out) {
            _output[_calcBiasIndex(LayerNum, outIndex)] = el; 
            ++outIndex;
        }
        ++LayerNum;
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
    
    for(auto& l :_parallelLayers) {
        if(!l.deserialize(ss)) {std::cout<<"ParallelDenseLayer internal layer error"<<std::endl;return false;}
    }
    
    if(ss.get() != '}') {std::cout<<"ParallelDenseLayer missing }"<<std::endl;return false;}
    if(ss.get() != '\n') {std::cout<<"DenseLayer missing line feed"<<std::endl;return false;}
    return true;
}
