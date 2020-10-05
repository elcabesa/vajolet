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

#include "layer.h"
#include "nnue_type.h"

class DifferentialList;
class FeatureList;

class Model {
public:
    Model();
    
    void addLayer(std::unique_ptr<Layer> l);
    Layer& getLayer(unsigned int index);
    unsigned int getLayerCount();
    
    nnueType forwardPass(const FeatureList& l, const FeatureList& h);
    nnueType incrementalPass(const DifferentialList& l, const DifferentialList& h);
    
    bool deserialize(std::ifstream& ss);
    
    void clear();

private:
    std::vector<std::unique_ptr<Layer>> _layers;
};

#endif
