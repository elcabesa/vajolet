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

#include "denseLayer.h"
#include "nnue_type.h"

class DifferentialList;
class FeatureList;

class Model {
public:
    Model();
    
    accumulatorType forwardPass(const FeatureList& l);
    accumulatorType incrementalPass(const DifferentialList& l);
    
    bool deserialize(std::istream& ss);
    static void init();

    void setBias(unsigned int layer, unsigned int n, float bias);
    void setWeight(unsigned int layer, unsigned int inN, unsigned int outN, float bias);
    
private:

    static std::vector<biasType> bias0;
    static std::vector<biasType> bias1;

    static std::vector<weightType> weight0;
    static std::vector<weightType> weight1;

    DenseLayer<outType, 768, 512> _layer0;
    DenseLayer<outType, 512, 1> _layer1;
};

#endif
