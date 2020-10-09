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

std::vector<flBiasType> Model::bias00;
std::vector<flBiasType> Model::bias01;
std::vector<biasType> Model::bias1;
std::vector<biasType> Model::bias2;
std::vector<biasType> Model::bias3;

std::vector<flWeightType> Model::weight00;
std::vector<flWeightType> Model::weight01;
std::vector<weightType> Model::weight1;
std::vector<weightType> Model::weight2;
std::vector<weightType> Model::weight3;

Model::Model():
    _layer0(&bias00, &bias01, &weight00, &weight01, 128, 128, 0),
    _layer1(&bias1, &weight1, 16384, 128, 9),
    _layer2(&bias2, &weight2, 2048, 64, 6),
    _layer3(&bias3, &weight3, 2048, 64, 0)
{
    bias00.resize(256, 0.0);
	bias01.resize(256, 0.0);
	bias1.resize(32, 0.0);
	bias2.resize(32, 0.0);
	bias3.resize(1, 0.0);
	
	weight00.resize(40960 * 256, 1.0);
	weight01.resize(40960 * 256, 1.0);
	weight1.resize(512 * 32, 1.0);
	weight2.resize(32 * 32, 1.0);
	weight3.resize(32 * 1, 1.0);
}

accumulatorType Model::forwardPass(const FeatureList& l, const FeatureList& h) {
    _layer0.propagate(l, h);
    _layer1.propagate(_layer0.output());
    _layer2.propagate(_layer1.output());
    return _layer3.propagateOut(_layer2.output(), 0, 0) * 4.8828;
}

accumulatorType Model::incrementalPass(const DifferentialList& l, const DifferentialList& h) {
    _layer0.incrementalPropagate(l, h);
    _layer1.propagate(_layer0.output());
    _layer2.propagate(_layer1.output());
    return _layer3.propagateOut(_layer2.output(), 0, 0) * 4.8828;
}

bool Model::deserialize(std::ifstream& ss) {
    ss.clear();
    ss.seekg(0);
    //std::cout<<"DESERIALIZE MODEL"<<std::endl;
    if(ss.get() != '{') {std::cout<<"MODEL missing {"<<std::endl;return false;}

    if(!_layer0.deserialize(ss)) {std::cout<<"MODEL internal layer0 error"<<std::endl;return false;}
    if(!_layer1.deserialize(ss)) {std::cout<<"MODEL internal layer1 error"<<std::endl;return false;}
    if(!_layer2.deserialize(ss)) {std::cout<<"MODEL internal layer2 error"<<std::endl;return false;}
    if(!_layer3.deserialize(ss)) {std::cout<<"MODEL internal layer3 error"<<std::endl;return false;}

    if(ss.get() != '}') {std::cout<<"MODEL missing }"<<std::endl;return false;} 
    return true;
}
