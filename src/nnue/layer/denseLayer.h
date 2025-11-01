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

#ifndef _DENSE_LAYER_H
#define _DENSE_LAYER_H

#include <vector>
#include "nnue_type.h"

class FeatureList;
class DifferentialList;

template <typename inputType, unsigned int inputSize, unsigned int outputSize> 
class DenseLayer{
public:

    DenseLayer(std::vector<biasType>* bias, std::vector<weightType>* weight, unsigned int outShift);
    ~DenseLayer();

    /*void propagate(const std::vector<inputType>& input);*/
    accumulatorType propagateOut(const std::vector<inputType>& input, const unsigned int index = 0, unsigned int o = 0);

    void propagate(const FeatureList& l);
    void incrementalPropagate(const DifferentialList& l);

    unsigned int _calcWeightIndex(const unsigned int i, const unsigned int o) const;

    bool deserialize(std::istream& ss);

    const std::vector<outType>& output() const;
    const std::vector<outType>& outputRelu() const;
#ifdef PRINTSTAT
    double getMinOut() const {return _minOut;}
    double getMaxOut() const {return _maxOut;}
#endif
    
private:
    unsigned int _inputSize;
    unsigned int _outputSize;

    unsigned int _outShift;
    std::vector<biasType>* _bias;
    std::vector<weightType>* _weight;

    std::vector<outType> _output;
    std::vector<outType> _outputRelu;

#ifdef PRINTSTAT
    static double _minOut;
    static double _maxOut;
#endif
    
};

#endif  
