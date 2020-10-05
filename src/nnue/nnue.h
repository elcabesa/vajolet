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
#include <array>

#include "bitBoardIndex.h"
#include "differentialList.h"
#include "featureList.h"
#include "score.h"
#include "model.h"
#include "tSquare.h"
#include "vajolet.h"
#include "nnue_type.h"

class Position;


class NNUE {
public:
    NNUE(const Position& pos);
    bool load(std::string path);
    void clear();
    Score eval();
    static bool loaded();
    std::set<unsigned int> createFeatures();
    void createWhiteFeatures(FeatureList& fl);
    void createBlackFeatures(FeatureList& fl);
    static unsigned int calcWhiteFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int calcBlackFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int whiteFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int blackFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int turnOffset(bool myturn);
    //void concatenateFeature(FeatureList f1, FeatureList f2, FeatureList& complete);
    static unsigned int mapWhitePiece(const bitboardIndex piece);
    static unsigned int mapBlackPiece(const bitboardIndex piece);

    bool whiteNoIncrementalEval;
    bool blackNoIncrementalEval;

    void removePiece(bitboardIndex piece, tSquare sq);
    void addPiece(bitboardIndex piece, tSquare sq);

    void clean();
private:
    Model _modelW;
    Model _modelB;
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
    Model _m;
#endif
    static bool _loaded;
    static std::vector<nnueType> bias00;
    static std::vector<nnueType> bias01;
    static std::vector<nnueType> bias1;
    static std::vector<nnueType> bias2;
    static std::vector<nnueType> bias3;

    static std::vector<nnueType> weight00;
    static std::vector<nnueType> weight01;
    static std::vector<nnueType> weight1;
    static std::vector<nnueType> weight2;
    static std::vector<nnueType> weight3;

    DifferentialList _whiteW;
    DifferentialList _blackW;
    DifferentialList _whiteB;
    DifferentialList _blackB;

    FeatureList _completeWhiteFeatureList;
    FeatureList _completeBlackFeatureList;
    //FeatureList _completeFeatureList;

    //unsigned int incrementalCount = 0;
    //unsigned int completeCount = 0;
    //unsigned int incrementalMaxSize = 0;

    Score _completeEval();
    Score _incrementalEval();

    const Position& _pos;
};



#endif
