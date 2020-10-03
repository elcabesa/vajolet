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

#ifndef _LAYER_H
#define _LAYER_H

#include <iostream>
#include <fstream>
#include "dense.h"

class Layer {
public:
    Layer(const unsigned int inputSize, const unsigned int outputSize);
    virtual ~Layer();
    
    unsigned int getInputSize() const;
    unsigned int getOutputSize() const;
    double getOutput(unsigned int i) const;
    const Input& output() const;
    void printOutput() const;
    
    virtual void propagate(const Input& input) = 0;
    virtual void incrementalPropagate(const Input& input) = 0;
    
    virtual bool deserialize(std::ifstream& ss) = 0;

protected:
    unsigned int _inputSize;
    unsigned int _outputSize;
    DenseInput _output;
    
};

#endif  
