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

#include <set>

#include "bitBoardIndex.h"
#include "linear.h"
#include "relu.h"
#include "score.h"
#include "model.h"
#include "tSquare.h"

class Position;

class NNUE {
public:
    NNUE();
    bool load(std::string path);
    void clear();
    Score eval(const Position& pos);
    static bool loaded();
    static std::set<unsigned int> createFeatures(const Position& pos);
    static std::set<unsigned int> createWhiteFeatures(const Position& pos);
    static std::set<unsigned int> createBlackFeatures(const Position& pos);
    static unsigned int calcWhiteFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int calcBlackFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int whiteFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int blackFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int turnOffset(bool myturn);
    static std::set<unsigned int> concatenateFeature(bool whiteTurn, std::set<unsigned int> w, std::set<unsigned int> b);
    static unsigned int mapWhitePiece(const bitboardIndex piece);
    static unsigned int mapBlackPiece(const bitboardIndex piece);
private:
    Model _model;
    static bool _loaded;
    static std::vector<double> bias00;
    static std::vector<double> bias01;
    static std::vector<double> bias1;
    static std::vector<double> bias2;
    static std::vector<double> bias3;

    static std::vector<double> weight00;
    static std::vector<double> weight01;
    static std::vector<double> weight1;
    static std::vector<double> weight2;
    static std::vector<double> weight3;

    linearActivation _linear;
	reluActivation _relu;


};

#endif
