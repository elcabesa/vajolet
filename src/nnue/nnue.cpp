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

#include <fstream>
#include <iostream>

#include "nnue.h"
#include "parallelDenseLayer.h"
#include "position.h"


bool NNUE::_loaded = false;

NNUE::NNUE(const Position& pos):_pos(pos){
    //std::cout<<"done"<<std::endl;
    clean();
}

bool NNUE::load(std::string path) {

    std::cout<<"reload NNUE parameters"<<std::endl;    
    std::cout<<"deserialize"<<std::endl;

    std::ifstream nnFile;
    nnFile.open(path);
    if(nnFile.is_open()) {
        if(_modelW.deserialize(nnFile) 
        ){
            _loaded = true;
            std::cout<<"done"<<std::endl;
            return true;
        }else {
            std::cout<<"FAIL"<<std::endl;
            return false;
        }
        nnFile.close();
    } else {
        std::cout<<"error deserializing NNUE file"<<std::endl;
        return false;
    }
}

void NNUE::close() {
    _loaded = false;
}

bool NNUE::loaded() {
    return _loaded;
}

Score NNUE::eval() {
    //std::cout<<"inc: "<<incrementalCount<< " complete: "<<completeCount<<std::endl;
    
    if (incrementalEvalDisabled()) {
        _resetCompleteEvalCondition();
        return _completeEval();
    }
    return _incrementalEval();
}

std::set<unsigned int> NNUE::features() {
    std::set<unsigned int> features;
    FeatureList white;
    FeatureList black;

    bool whiteTurn = _pos.isWhiteTurn();
    bool blackTurn = _pos.isBlackTurn();

    _createWhiteFeatures(white);
    _createBlackFeatures(black);


    for(unsigned int i = 0; i< white.size(); ++i) {
        features.insert(_turnOffset(whiteTurn) + white.get(i));
    }

    for(unsigned int i = 0; i< black.size(); ++i) {
        features.insert(_turnOffset(blackTurn) + black.get(i));
    }
    return features;
}

void NNUE::clean() {
    disableWhiteIncrementalEval();
    disableBlackIncrementalEval();

    _whiteW.clear();
    _blackW.clear();
    _whiteB.clear();
    _blackB.clear();

    _completeWhiteFeatureList.clear();
    _completeBlackFeatureList.clear();
}

void NNUE::disableWhiteIncrementalEval() {
    _whiteNoIncrementalEval = true;
}

void NNUE::disableBlackIncrementalEval() {
    _blackNoIncrementalEval = true;
}

bool NNUE::incrementalEvalDisabled() const {
    return _whiteNoIncrementalEval || _blackNoIncrementalEval;
}

void NNUE::removePiece(bitboardIndex piece, tSquare sq) {
    const unsigned int CompleteEvalThreshold = 50;

    auto wf = _whiteFeature(_mapWhitePiece(piece), sq, _pos.getSquareOfThePiece(bitboardIndex::whiteKing));
    auto bf = _blackFeature(_mapBlackPiece(piece), sq, _pos.getSquareOfThePiece(bitboardIndex::blackKing));

    _whiteW.remove(wf);
    _blackW.remove(bf);
    _whiteB.remove(wf);
    _blackB.remove(bf);

    if (_whiteW.size() + _blackW.size() > CompleteEvalThreshold) { disableWhiteIncrementalEval(); }
    if (_whiteB.size() + _blackB.size() > CompleteEvalThreshold) { disableBlackIncrementalEval(); }
}

void NNUE::addPiece(bitboardIndex piece, tSquare sq) {
    const unsigned int CompleteEvalThreshold = 50;

    auto wf = _whiteFeature(_mapWhitePiece(piece), sq, _pos.getSquareOfThePiece(bitboardIndex::whiteKing));
    auto bf = _blackFeature(_mapBlackPiece(piece), sq, _pos.getSquareOfThePiece(bitboardIndex::blackKing));

    _whiteW.add(wf);
    _blackW.add(bf);
    _whiteB.add(wf);
    _blackB.add(bf);

    if (_whiteW.size() + _blackW.size() > CompleteEvalThreshold) { disableWhiteIncrementalEval(); }
    if (_whiteB.size() + _blackB.size() > CompleteEvalThreshold) { disableBlackIncrementalEval(); }
}

void NNUE::_createBlackFeatures(FeatureList& fl){
    bitboardIndex blackPow[10] = {
        blackQueens,
        blackRooks,
        blackBishops,
        blackKnights,
        blackPawns,
        whiteQueens,
        whiteRooks,
        whiteBishops,
        whiteKnights,
        whitePawns
    };
    
    tSquare bkSq = _pos.getSquareOfThePiece(bitboardIndex::blackKing);
    for(unsigned int piece = 0; piece < 10; ++piece) {
        
        bitMap b = _pos.getBitmap(blackPow[piece]);
        while(b)
        {
            tSquare pieceSq = iterateBit(b);
            fl.add(_blackFeature(piece, pieceSq, bkSq));
        }
    }
}

void NNUE::_createWhiteFeatures(FeatureList& fl){
    bitboardIndex whitePow[10] = {
        whiteQueens,
        whiteRooks,
        whiteBishops,
        whiteKnights,
        whitePawns,
        blackQueens,
        blackRooks,
        blackBishops,
        blackKnights,
        blackPawns
    };

    tSquare wkSq = _pos.getSquareOfThePiece(bitboardIndex::whiteKing);
    for(unsigned int piece = 0; piece < 10; ++piece) {

        bitMap b = _pos.getBitmap(whitePow[piece]);
        while(b)
        {
            tSquare pieceSq = iterateBit(b);
            fl.add(_whiteFeature(piece, pieceSq, wkSq));
        }
    }
}

unsigned int NNUE::_whiteFeature(unsigned int  piece, tSquare pSquare, tSquare ksq) {
    unsigned int f = piece + (10 * pSquare) + (640 * ksq);
    return f;
}

unsigned int NNUE::_blackFeature(unsigned int  piece, tSquare pSquare, tSquare ksq) {
    unsigned int f = piece + (10 * (pSquare ^ 56)) + (640 * (ksq ^ 56));
    return f;
}

unsigned int NNUE::_turnOffset(bool myTurn) {
    return myTurn ? 0 : 40960;
}

unsigned int NNUE::_mapWhitePiece(const bitboardIndex piece) {
    unsigned int n[] = {
        0, //occupiedSquares,			//0		00000000
        0, //whiteKing,					//1		00000001
        0, //whiteQueens,				//2		00000010
        1, //whiteRooks,				//3		00000011
        2, //whiteBishops,				//4		00000100
        3, //whiteKnights,				//5		00000101
        4, //whitePawns,				//6		00000110
        0, //whitePieces,				//7		00000111

        0, //separationBitmap,			//8
        0, //blackKing,					//9		00001001
        5, //blackQueens,				//10	00001010
        6, //blackRooks,				//11	00001011
        7, //blackBishops,				//12	00001100
        8, //blackKnights,				//13	00001101
        9, //blackPawns,				//14	00001110
        0  //blackPieces,				//15	00001111
    };
    return n[piece];
}

unsigned int NNUE::_mapBlackPiece(const bitboardIndex piece) {
    unsigned int n[] = {
        0, //occupiedSquares,			//0		00000000
        0, //whiteKing,					//1		00000001
        5, //whiteQueens,				//2		00000010
        6, //whiteRooks,				//3		00000011
        7, //whiteBishops,				//4		00000100
        8, //whiteKnights,				//5		00000101
        9, //whitePawns,				//6		00000110
        0, //whitePieces,				//7		00000111

        0, //separationBitmap,			//8
        0, //blackKing,					//9		00001001
        0, //blackQueens,				//10	00001010
        1, //blackRooks,				//11	00001011
        2, //blackBishops,				//12	00001100
        3, //blackKnights,				//13	00001101
        4, //blackPawns,				//14	00001110
        0  //blackPieces,				//15	00001111
    };
    return n[piece];
}

Score NNUE::_completeEval() {
    Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
    //++completeCount;
    //std::cout<<"no incremental eval"<<std::endl;
    // TODO if we move one king calculate everything from scratch
    // can we do it faster??
    _completeWhiteFeatureList.clear();
    _completeBlackFeatureList.clear();
    _createWhiteFeatures(_completeWhiteFeatureList);
    _createBlackFeatures(_completeBlackFeatureList);

    _whiteW.clear();
    _blackW.clear();
    _whiteB.clear();
    _blackB.clear();

    Score scoreW = _modelW.forwardPass(_completeWhiteFeatureList, _completeBlackFeatureList);
    Score scoreB = _modelB.forwardPass(_completeBlackFeatureList, _completeWhiteFeatureList);

    Score score;
    if(_pos.isWhiteTurn()) {
        score = scoreW;
    } else {
        score = scoreB;
    }
    score = std::min(highSat,score);
    score = std::max(lowSat,score);
    return score;
}

Score NNUE::_incrementalEval() {
    const unsigned int CompleteEvalThreshold = 50;
    //++incrementalCount;
    Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
    Score score;
    if(_pos.isWhiteTurn()) {
        if(_whiteW.size() + _blackW.size() > CompleteEvalThreshold) {return _completeEval();}

        score = _modelW.incrementalPass(_whiteW, _blackW);
        _whiteW.clear();
        _blackW.clear();
    } else {
        if(_whiteB.size() + _blackB.size() > CompleteEvalThreshold) {return _completeEval();}

        score = _modelB.incrementalPass(_blackB, _whiteB);
        _whiteB.clear();
        _blackB.clear();

    }

//std::cout<<"incrementalMaxSize " << incrementalMaxSize << std::endl;
	
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
    _completeWhiteFeatureList.clear();
    _completeBlackFeatureList.clear();
    createWhiteFeatures(_completeWhiteFeatureList);
    createBlackFeatures(_completeBlackFeatureList);
    Score score2;
    if(_pos.isWhiteTurn()) {
        score2 = _m.forwardPass(_completeWhiteFeatureList, _completeBlackFeatureList);
    } else {
        score2 = _m.forwardPass(_completeBlackFeatureList, _completeWhiteFeatureList);
    }
    if(score2 != score) {
        std::cout<<"AHHHHHHHHHHHHHHHHHHHHH"<<std::endl;
}
#endif

    score = std::min(highSat,score);
    score = std::max(lowSat,score);
    return score;
}

void NNUE::_resetCompleteEvalCondition() {
    _whiteNoIncrementalEval = false;
    _blackNoIncrementalEval = false;
}