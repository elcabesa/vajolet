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
#include <fstream>
#include <iostream>

#include "model.h"

std::vector<biasType> Model::bias0;
std::vector<biasType> Model::bias1;


std::vector<weightType> Model::weight0;
std::vector<weightType> Model::weight1;

void Model::init() {
    bias0.resize(512, 0.0);
	bias1.resize(1, 0.0);
	
	weight0.resize(768 * 512, 1.0);
	weight1.resize(512 * 1, 1.0);

}

Model::Model():
    _layer0(&bias0, &weight0, 0),
    _layer1(&bias1, &weight1, 0)
{}

void Model::setBias(unsigned int layer, unsigned int n, float bias) {
    std::vector<biasType>* l;
    if(layer == 0) {
        l = &bias0;
    } else {
        l = &bias1;
    }
    (*l)[n] = bias;
}

void Model::setWeight(unsigned int layer, unsigned int inN, unsigned int outN, float weight) {
    if(layer == 0) {
        weight0[_layer0._calcWeightIndex(inN, outN)] = weight;
    } else {
        weight1[_layer1._calcWeightIndex(inN, outN)] = weight;
    }
}

accumulatorType Model::forwardPass(const FeatureList& l) {
    _layer0.propagate(l);
    return _layer1.propagateOut(_layer0.outputRelu(), 0, 0);
}

accumulatorType Model::incrementalPass(const DifferentialList& l) {
    _layer0.incrementalPropagate(l);
    return _layer1.propagateOut(_layer0.outputRelu(), 0, 0);
}

bool Model::deserialize(std::istream& ss) {
    ss.clear();

    if(!_layer0.deserialize(ss)) {std::cout<<"MODEL internal layer0 error"<<std::endl;return false;}
    if(!_layer1.deserialize(ss)) {std::cout<<"MODEL internal layer1 error"<<std::endl;return false;}

    return true;
}


void Model::printMinMax() const {
#ifdef PRINTSTAT
    std::cout<<"NNUE_STATS:"<<std::endl;
    std::cout<<_layer0.getMinOut() <<" "<< _layer0.getMaxOut()<<std::endl;
    std::cout<<_layer1.getMinOut() <<" "<< _layer1.getMaxOut()<<std::endl;
#endif
}

