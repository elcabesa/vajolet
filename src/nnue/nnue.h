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

class Position;

//#define CHECK_NNUE_FEATURE_EXTRACTION


class NNUE {
public:
    NNUE(const Position& pos);

    bool load(std::string path);
    void close();
    static bool loaded();

    Score eval();
    Score eval(FeatureList fl);

    void disableIncrementalEval(); // evaluate if it's still necessary for king move and castling
    bool incrementalEvalDisabled() const;
    
    void removePiece(bitboardIndex piece, tSquare sq);
    void addPiece(bitboardIndex piece, tSquare sq);

    void clean();
    
    std::set<unsigned int> features();

    static tSquare _getSquareFromFeature(unsigned int);
    static bitboardIndex _getPieceFromFeature(unsigned int);

private:
    Model _model;

    static bool _loaded;
    const unsigned int _completeEvalThreshold = 20;

    DifferentialList _diffFeatureList;

    FeatureList _completeFeatureList;

    void _createFeatures(FeatureList& fl);

    static unsigned int _feature(unsigned int piece, tSquare pSquare);
    static unsigned int _mapPiece(const bitboardIndex piece);



    void _resetCompleteEvalCondition();
    bool _noIncrementalEval;

    Score _completeEval();
    Score _incrementalEval();

    const Position& _pos;
};



#endif
