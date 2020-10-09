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

#ifndef _MODEL_H
#define _MODEL_H

#include <memory>
#include <vector>

#include "parallelDenseLayer.h"
#include "denseLayer.h"
#include "nnue_type.h"

class DifferentialList;
class FeatureList;

class Model {
public:
    Model();
    
    accumulatorType forwardPass(const FeatureList& l, const FeatureList& h);
    accumulatorType incrementalPass(const DifferentialList& l, const DifferentialList& h);
    
    bool deserialize(std::ifstream& ss);
    
private:

    static std::vector<biasType> bias00;
    static std::vector<biasType> bias01;
    static std::vector<biasType> bias1;
    static std::vector<biasType> bias2;
    static std::vector<biasType> bias3;

    static std::vector<weightType> weight00;
    static std::vector<weightType> weight01;
    static std::vector<weightType> weight1;
    static std::vector<weightType> weight2;
    static std::vector<weightType> weight3;

    //std::vector<std::unique_ptr<Layer>> _layers;
    ParallelDenseLayer _layer0;
    DenseLayer _layer1;
    DenseLayer _layer2;
    DenseLayer _layer3;
};

#endif
