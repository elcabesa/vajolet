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

#include "layer.h"

Layer::Layer(const unsigned int inputSize, const unsigned int outputSize, unsigned int biasScale, unsigned int weightScale, unsigned int outShift):
    _inputSize(inputSize),
    _outputSize(outputSize),
    _biasScale(biasScale),
    _weightScale(weightScale),
    _outShift(outShift)
{}

Layer::~Layer() {}

unsigned int Layer::getInputSize() const {
    return _inputSize;
}

unsigned int Layer::getOutputSize() const {
    return _outputSize;
}



