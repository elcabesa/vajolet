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
    bias0.resize(32, 0.0);
	bias1.resize(1, 0.0);
	
	weight0.resize(768 * 32, 1.0);
	weight1.resize(32 * 1, 1.0);

}

Model::Model():
    _layer0(&bias0, &weight0, 0),
    _layer1(&bias1, &weight1, 0)
{}

accumulatorType Model::forwardPass(const FeatureList& l) {
    _layer0.propagate(l);
    return _layer1.propagateOut(_layer0.output(), 0, 0);
}

/*void Model::calcFirstLayer(const FeatureList& l, const FeatureList& h) {
    _layer0.propagate(l, h);
}*/

accumulatorType Model::incrementalPass(const DifferentialList& l) {
    _layer0.incrementalPropagate(l);
    return _layer1.propagateOut(_layer0.output(), 0, 0);
}

#define VERSION "0004"
bool Model::deserialize(std::ifstream& ss) {
    ss.clear();
    ss.seekg(0);
    //std::cout<<"DESERIALIZE MODEL"<<std::endl;
    if(ss.get() != '{') {std::cout<<"MODEL missing {"<<std::endl;return false;}
    if(ss.get() != 'V') {std::cout<<"MODEL missing V"<<std::endl;return false;}
    if(ss.get() != ':') {std::cout<<"MODEL missing :"<<std::endl;return false;}
    char buffer[4];
    ss.read(buffer, 4);
    std::string v(buffer, 4);
    if(v != VERSION) {std::cout<<"WRONG NETWORK VERSION: "<<v<<" expected: "<<VERSION<<std::endl;return false;}
    if(ss.get() != '}') {std::cout<<"MODEL missing }"<<std::endl;return false;}

    if(ss.get() != '{') {std::cout<<"MODEL missing {"<<std::endl;return false;}

    if(!_layer0.deserialize(ss)) {std::cout<<"MODEL internal layer0 error"<<std::endl;return false;}
    if(!_layer1.deserialize(ss)) {std::cout<<"MODEL internal layer1 error"<<std::endl;return false;}

    if(ss.get() != '}') {std::cout<<"MODEL missing }"<<std::endl;return false;} 
    return true;
}
