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


template <unsigned int inputSize, unsigned int outputSize> 
class ParallelDenseLayer {
public:
    ParallelDenseLayer(std::vector<flBiasType>* bias,std::vector<flWeightType>* weight, unsigned int outShift);
    ~ParallelDenseLayer();

    void propagate(const FeatureList& l, const FeatureList& h);
    void incrementalPropagate(const DifferentialList& l, const DifferentialList& h);
    
    bool deserialize(std::ifstream& ss);
    const std::vector<flOutType>& output() const;
    
    
private:
    unsigned int _calcWeightIndex(const unsigned int i, const unsigned int o) const;
    bool _deserialize(std::ifstream& ss, std::vector<flBiasType>* bias, std::vector<flWeightType>* weight);

    const unsigned int _inputSize;
    const unsigned int _outputSize;

    const unsigned int _outShift;

    const unsigned int _layerOutputSize;

    std::vector<flBiasType>* _bias;
    std::vector<flWeightType>* _weight;

    /*double _max = -127;
    double _min = 127;*/
    std::vector<flOutType> _accumulator;
    std::vector<flOutType> _output;

    

};

#endif  
