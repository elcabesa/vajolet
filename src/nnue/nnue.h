/*
	This file is part of Vajolet.

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
#ifndef NNUE_H_
#define NNUE_H_ 

#include <vector>

#include "score.h"
#include "model.h"

class Position;

class NNUE {
public:
    NNUE();
    void init();
    Score eval(const Position& pos);
    bool loaded() const;
    
private:
    std::vector<unsigned int> createFeatures(const Position& pos) const;
    Model _model;
    bool _loaded;

};

#endif
