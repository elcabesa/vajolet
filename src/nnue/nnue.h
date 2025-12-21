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
#include <memory>

#include "bitBoardIndex.h"
#include "differentialList.h"
#include "featureList.h"
#include "score.h"
#include "tSquare.h"
#include "vajolet.h"

class Position;
class Model;

//#define CHECK_NNUE_FEATURE_EXTRACTION


class NNUE {
public:
    NNUE(const Position& pos);

    enum perspective {
        whitePow,
        blackPow
    };

    bool load(std::string path);
    void close();
    static bool loaded();

    Score eval();
    Score eval(FeatureList fl);

    bool incrementalEvalDisabled() const;
    
    void removePiece(bitboardIndex piece, tSquare sq);
    void addPiece(bitboardIndex piece, tSquare sq);

    void clean();
    
    std::set<unsigned int> features();

    static tSquare _getSquareFromFeature(unsigned int);
    static bitboardIndex _getPieceFromFeature(unsigned int);

    void printStats();

private:

    std::shared_ptr<Model> _model;

#ifdef CHECK_NNUE_FEATURE_EXTRACTION
    std::shared_ptr<Model> _modelCheck;
#endif

    static bool _loaded;
    const unsigned int _completeEvalThreshold = 20;

    DifferentialList _diffFeatureListW;
    DifferentialList _diffFeatureListB;

    FeatureList _completeFeatureListW;
    FeatureList _completeFeatureListB;

    void _createFeatures(FeatureList& fl, perspective p);

    static unsigned int _feature(unsigned int piece, tSquare pSquare, perspective p);
    static unsigned int _mapPiece(const bitboardIndex piece, perspective p);

    void _disableIncrementalEval(); // evaluate if it's still necessary for king move and castling


    void _resetCompleteEvalCondition();
    bool _noIncrementalEval;

    Score _completeEval();
    Score _incrementalEval();

    const Position& _pos;
};



#endif
