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

#include "bitBoardIndex.h"
#include "score.h"
#include "model.h"
#include "tSquare.h"

class Position;

class NNUE {
public:
    NNUE();
    bool init(std::string path);
    void clear();
    Score eval(const Position& pos);
    bool loaded() const;
    static std::vector<unsigned int> createFeatures(const Position& pos);
    static unsigned int calcWhiteFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int calcBlackFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
private:
    Model _model;
    bool _loaded;

};

#endif
