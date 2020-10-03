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

#include "layer.h"
#include "denseLayer.h"

class Activation;

class ParallelDenseLayer: public Layer {
public:
    ParallelDenseLayer(const unsigned int number, const unsigned int inputSize, const unsigned int outputSize, std::vector<std::vector<double>*> biases, std::vector<std::vector<double>*> weights);
    ~ParallelDenseLayer();
    
    void propagate(const FeatureList& input);
    void propagate(const FeatureList& l, const FeatureList& h);
    void incrementalPropagate(const DifferentialList& input);
    void incrementalPropagate(const DifferentialList& l, const DifferentialList& h);

    void propagate(const std::vector<double>& input);
    void incrementalPropagate(const std::vector<double>& input);
    
    DenseLayer& getLayer(unsigned int);

    unsigned int _calcBiasIndex(const unsigned int layer, const unsigned int offset) const;
    
    bool deserialize(std::ifstream& ss);
    
private:
    std::vector<DenseLayer> _parallelLayers;
    const unsigned int _layerInputSize;
    const unsigned int _layerOutputSize;

};

#endif  
