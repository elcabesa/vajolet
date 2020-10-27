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


class NNUE {
public:
    NNUE(const Position& pos);

    bool load(std::string path);
    void close();
    static bool loaded();

    Score eval();

    void disableWhiteIncrementalEval();
    void disableBlackIncrementalEval();
    bool incrementalEvalDisabled() const;
    
    void removePiece(bitboardIndex piece, tSquare sq);
    void addPiece(bitboardIndex piece, tSquare sq);

    void clean();
    
    std::set<unsigned int> features();
private:
    Model _modelW;
    Model _modelB;
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
    Model _m;
#endif
    static bool _loaded;

    DifferentialList _whiteW;
    DifferentialList _blackW;
    DifferentialList _whiteB;
    DifferentialList _blackB;

    FeatureList _completeWhiteFeatureList;
    FeatureList _completeBlackFeatureList;

    //unsigned int incrementalCount = 0;
    //unsigned int completeCount = 0;
    //unsigned int incrementalMaxSize = 0;

    void _createWhiteFeatures(FeatureList& fl);
    void _createBlackFeatures(FeatureList& fl);

    //static unsigned int _calcWhiteFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    //static unsigned int _calcBlackFeature(bool whiteTurn, unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int _whiteFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int _blackFeature(unsigned int piece, tSquare pSquare, tSquare ksq);
    static unsigned int _turnOffset(bool myturn);
    static unsigned int _mapWhitePiece(const bitboardIndex piece);
    static unsigned int _mapBlackPiece(const bitboardIndex piece);

    void _resetCompleteEvalCondition();
    bool _whiteNoIncrementalEval;
    bool _blackNoIncrementalEval;

    Score _completeEval();
    Score _incrementalEval();

    const Position& _pos;
};



#endif
