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

#ifndef _PARALLEL_DENSE_LAYER_H
#define _PARALLEL_DENSE_LAYER_H

#include <vector>

#include "denseLayer.h"
#include "nnue_type.h"

class Activation;

class ParallelDenseLayer {
public:
    ParallelDenseLayer(const unsigned int inputSize, const unsigned int outputSize, std::vector<biasType>* bias0, std::vector<biasType>* bias1, std::vector<weightType>* weight0, std::vector<weightType>* weight1, unsigned int biasScale, unsigned int weightScale, unsigned int outShift);
    ~ParallelDenseLayer();

    void propagate(const FeatureList& l, const FeatureList& h);
    void incrementalPropagate(const DifferentialList& l, const DifferentialList& h);
    
    bool deserialize(std::ifstream& ss);
    const std::vector<flOutType>& output() const;
    
    
private:
    unsigned int _calcWeightIndex(const unsigned int i, const unsigned int o) const;
    bool _deserialize(std::ifstream& ss, std::vector<biasType>* bias, std::vector<weightType>* weight);

    const unsigned int _inputSize;
    const unsigned int _outputSize;

    const unsigned int _biasScale;
    const unsigned int _weightScale;
    const unsigned int _outShift;

    const unsigned int _layerOutputSize;

    std::vector<biasType>* _bias0;
    std::vector<biasType>* _bias1;
    std::vector<weightType>* _weight0;
    std::vector<weightType>* _weight1;

    /*double _max = -127;
    double _min = 127;*/
    std::vector<accumulatorType> _accumulator;
    std::vector<flOutType> _output;

    

};

#endif  
