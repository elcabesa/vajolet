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

#include "model.h"

Model::Model(){}

void Model::addLayer(std::unique_ptr<Layer> l) {
    if(_layers.size() > 0 && _layers.back()->getOutputSize() != l->getInputSize()) {
        std::cerr<<"ERROR in SIZING"<< std::endl;
        exit(0);
    }
        
    _layers.push_back(std::move(l));
}

nnueType Model::forwardPass(const FeatureList& l, const FeatureList& h) {
    const std::vector<nnueType>* in;
    unsigned int i= 0;
    for(auto& p: _layers) {
        if(i == 0) {
            p->propagate(l, h);
        } else {
            p->propagate(*in);
        }
        in = &p->output();
        ++i;
    }
    return (*in)[0] * 10000;
}

nnueType Model::incrementalPass(const DifferentialList& l, const DifferentialList& h) {
    const std::vector<nnueType>* in;
    unsigned int i= 0;
    for(auto& p: _layers) {
        if(i == 0) {
            p->incrementalPropagate(l, h);
        } else { 
            p->propagate(*in);
        }
        in = &p->output();
        ++i;
    }
    return (*in)[0] * 10000;
}

bool Model::deserialize(std::ifstream& ss) {
    ss.clear();
    ss.seekg(0);
    //std::cout<<"DESERIALIZE MODEL"<<std::endl;
    if(ss.get() != '{') {std::cout<<"MODEL missing {"<<std::endl;return false;}
    for(auto& l :_layers) {
        if(!l->deserialize(ss)) {std::cout<<"MODEL internal layer error"<<std::endl;return false;}
    }
    if(ss.get() != '}') {std::cout<<"MODEL missing }"<<std::endl;return false;} 
    return true;
}

Layer& Model::getLayer(unsigned int index) {
    assert(index < getLayerCount());
    return *(_layers[index]);
}
unsigned int Model::getLayerCount() {
    return _layers.size();
}

void Model::clear() {
    _layers.clear();
}
