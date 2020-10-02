#include <cassert>
#include <iostream>
#include <numeric>

#include "activation.h"
#include "labeledExample.h"
#include "model.h"

Model::Model(){}

void Model::addLayer(std::unique_ptr<Layer> l) {
    if(_layers.size() > 0 && _layers.back()->getOutputSize() != l->getInputSize()) {
        std::cerr<<"ERROR in SIZING"<< std::endl;
        exit(0);
    }
        
    _layers.push_back(std::move(l));
}

const Input& Model::forwardPass(const Input& input, bool verbose /* = false */) {
    const Input* in = &input;
    for(auto& p: _layers) {
        p->propagate(*in);
        if(verbose) {
            p->printOutput();
        }
        in = &p->output();
    }
    if(verbose) {
        in->print();
    }
    return *in;
}

bool Model::deserialize(std::ifstream& ss) {
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
