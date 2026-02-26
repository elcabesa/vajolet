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
    bias0.resize(accumulatorSize, 0.0);
    bias1.resize(outSize * outputBuckets, 0.0);
	
    weight0.resize(inputSize * accumulatorSize, 1.0);
    weight1.resize(accumulatorSize * 2 * outSize * outputBuckets, 1.0);

}

Model::Model(outType scaleFl, outType scaleSl):
    _layer0W(&bias0, &weight0, 1, scaleFl, 1),
    _layer0B(&bias0, &weight0, 1, scaleFl, 1),
    _layer1(&bias1, &weight1, scaleFl, scaleSl, outputBuckets)
{}

unsigned int Model::calcBucket(unsigned int pieceCount) {
    return (pieceCount - 2) / _bucketDivisor;
}

void Model::printStats() const {
#ifdef CALC_DEBUG_DATA
    _layer0W.printStat();
    _layer0B.printStat();
    _layer1.printStat();
#endif
}


accumulatorTypeOut Model::forwardPass(const FeatureList& lw,const FeatureList& lb, NNUE::perspective p, unsigned int pieceCount) {
    _layer0W.propagate(lw);
    _layer0B.propagate(lb);

    return p == NNUE::whitePow ?
        _layer1.propagateOut(_layer0W.outputScRelu(), _layer0B.outputScRelu(), calcBucket(pieceCount)) :
        _layer1.propagateOut(_layer0B.outputScRelu(), _layer0W.outputScRelu(), calcBucket(pieceCount));
}

accumulatorTypeOut Model::incrementalPass(const DifferentialList& lw, const DifferentialList& lb, NNUE::perspective p, unsigned int pieceCount) {
    _layer0W.incrementalPropagate(lw);
    _layer0B.incrementalPropagate(lb);

    return p == NNUE::whitePow ?
        _layer1.propagateOut(_layer0W.outputScRelu(), _layer0B.outputScRelu(), calcBucket(pieceCount)) :
        _layer1.propagateOut(_layer0B.outputScRelu(), _layer0W.outputScRelu(), calcBucket(pieceCount));
}

bool Model::deserialize(std::istream& ss) {
    ss.clear();

    if(!_layer0W.deserialize(ss)) {std::cout<<"MODEL internal layer0 error"<<std::endl;return false;}
    if(!_layer1.deserialize(ss)) {std::cout<<"MODEL internal layer1 error"<<std::endl;return false;}

    return true;
}
