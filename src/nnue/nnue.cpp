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
#include "position.h"
#include "incbin.h"

INCBIN(NnueInternalFile, "../../NN/nnue.par");


bool NNUE::_loaded = false;

NNUE::NNUE(const Position& pos):_model(_scale),_pos(pos) {
    //std::cout<<"done"<<std::endl;
    clean();
}

bool NNUE::load(std::string path) {

    std::cout<<"reload NNUE parameters"<<std::endl;    
    std::cout<<"deserialize"<<std::endl;

    std::cout<<"load"<<std::endl;

    if(path=="internal") {
        std::cout<<"internal"<<std::endl;

        class MemoryBuffer: public std::basic_streambuf<char> {
        public:
            MemoryBuffer(char* p, size_t n) {
                setg(p, p, p + n);
                setp(p, p + n);
            }
        };

        MemoryBuffer buffer(const_cast<char*>(reinterpret_cast<const char*>(gNnueInternalFileData)),
                            size_t(gNnueInternalFileSize));

        std::istream nnFile(&buffer);
        if(_model.deserialize(nnFile)){
            _loaded = true;
            std::cout<<"done"<<std::endl;
            return true;
        }else {
            std::cout<<"FAIL"<<std::endl;
            return false;
        }
    }
    else
    {
        std::ifstream nnFile;
        nnFile.open(path);
        if(nnFile.is_open()) {
            if(_model.deserialize(nnFile)
            ){
                _loaded = true;
                std::cout<<"done"<<std::endl;
                nnFile.close();
                return true;
            }else {
                std::cout<<"FAIL"<<std::endl;
                nnFile.close();
                return false;
            }

        } else {
            std::cout<<"error deserializing NNUE file"<<std::endl;
            return false;
        }
    }
}

void NNUE::close() {
    std::cout<<"destroy"<<std::endl;
    _loaded = false;
}

bool NNUE::loaded() {
    return _loaded;
}

Score NNUE::eval() {

    //std::cout<<"inc: "<<incrementalCount<< " complete: "<<completeCount<<std::endl;
    Score s;
    if (incrementalEvalDisabled()) {
        _resetCompleteEvalCondition();
        s =  _completeEval();
    } else {
        s = _incrementalEval();
    }
    if(_pos.isBlackTurn()) {
        return -s;
    } else {
        return s;
    }
}

Score NNUE::eval(FeatureList fl) {

    Score lowSat = -SCORE_KNOWN_WIN;
    Score highSat = SCORE_KNOWN_WIN;

    auto s  = _model.forwardPass(fl); //Q24

    Score score = s * (50000.0f / (_scale*_scale) );
    score = std::min(highSat, score);
    score = std::max(lowSat, score);
    return score;
}

std::set<unsigned int> NNUE::features() {
    std::set<unsigned int> features;
    FeatureList fl;

    _createFeatures(fl);

    for(unsigned int i = 0; i< fl.size(); ++i) {
        features.insert(fl.get(i));
    }

    return features;
}

void NNUE::clean() {
    disableIncrementalEval();

    _diffFeatureList.clear();

    _completeFeatureList.clear();
}

void NNUE::disableIncrementalEval() {
    _noIncrementalEval = true;
}


bool NNUE::incrementalEvalDisabled() const {
    return _noIncrementalEval;
}

void NNUE::removePiece(bitboardIndex piece, tSquare sq) {
    auto f = _feature(_mapPiece(piece), sq);
    _diffFeatureList.remove(f);
    if (_diffFeatureList.size() >= _completeEvalThreshold) { disableIncrementalEval(); }
}

void NNUE::addPiece(bitboardIndex piece, tSquare sq) {
    auto f = _feature(_mapPiece(piece), sq);
    _diffFeatureList.add(f);
    if (_diffFeatureList.size() >= _completeEvalThreshold) { disableIncrementalEval(); }
}

void NNUE::_createFeatures(FeatureList& fl){
    bitboardIndex whitePow[12] = {
        whiteKing,
        whiteQueens,
        whiteRooks,
        whiteBishops,
        whiteKnights,
        whitePawns,
        blackKing,
        blackQueens,
        blackRooks,
        blackBishops,
        blackKnights,
        blackPawns
    };

    for(unsigned int piece = 0; piece < 12; ++piece) {

        bitMap b = _pos.getBitmap(whitePow[piece]);
        while(b)
        {
            tSquare pieceSq = iterateBit(b);
            fl.add(_feature(piece, pieceSq));
        }
    }
}

unsigned int NNUE::_feature(unsigned int  piece, tSquare pSquare) {
    unsigned int f = (piece * 64) + pSquare;
    return f;
}

tSquare NNUE::_getSquareFromFeature(unsigned int f) {return tSquare(f&63);}
bitboardIndex NNUE::_getPieceFromFeature(unsigned int f) {
    bitboardIndex whitePow[12] = {
        whiteKing,
        whiteQueens,
        whiteRooks,
        whiteBishops,
        whiteKnights,
        whitePawns,
        blackKing,
        blackQueens,
        blackRooks,
        blackBishops,
        blackKnights,
        blackPawns
    };
    return whitePow[f>>6];}


unsigned int NNUE::_mapPiece(const bitboardIndex piece) {
    unsigned int n[] = {
        0, //occupiedSquares,			//0		00000000
        0, //whiteKing,					//1		00000001
        1, //whiteQueens,				//2		00000010
        2, //whiteRooks,				//3		00000011
        3, //whiteBishops,				//4		00000100
        4, //whiteKnights,				//5		00000101
        5, //whitePawns,				//6		00000110
        0, //whitePieces,				//7		00000111

        0, //separationBitmap,			//8
        6, //blackKing,					//9		00001001
        7, //blackQueens,				//10	00001010
        8, //blackRooks,				//11	00001011
        9, //blackBishops,				//12	00001100
        10, //blackKnights,				//13	00001101
        11, //blackPawns,				//14	00001110
        0  //blackPieces,				//15	00001111
    };
    return n[piece];
}

Score NNUE::_completeEval() {
    //std::cout<< "complete eval"<<std::endl;

    Score lowSat = -SCORE_KNOWN_WIN;
    Score highSat = SCORE_KNOWN_WIN;

    _diffFeatureList.clear();// todo remove?? da verificare se è più veloce o meno, non dovrebbe cambioare il numero di nodi ma solo nps

    _completeFeatureList.clear();
    _createFeatures(_completeFeatureList);

    auto s  = _model.forwardPass(_completeFeatureList);

    Score score = s * (50000.0f / (_scale*_scale));
    score = std::min(highSat, score);
    score = std::max(lowSat, score);
    return score;
}

Score NNUE::_incrementalEval() {

    //++incrementalCount;
    Score lowSat = -SCORE_KNOWN_WIN;
    Score highSat = SCORE_KNOWN_WIN;

    if(_diffFeatureList.size() >= _completeEvalThreshold) {return _completeEval();}

    Score score = _model.incrementalPass(_diffFeatureList) * (50000.0f / (_scale*_scale));
    _diffFeatureList.clear();

    score = std::min(highSat,score);
    score = std::max(lowSat,score);

#ifdef CHECK_NNUE_FEATURE_EXTRACTION
    FeatureList fl;
    fl.clear();
    _createFeatures(fl);
    Score score2 = _model.forwardPass(fl) (50000.0f / (_scale*_scale));
    score2 = std::min(highSat, score2);
    score2 = std::max(lowSat, score2);

    if(std::abs(score2 - score) > 1.0) {
        std::cout<<"AHHHHHHHHHHHHHHHHHHHHH "<<std::abs(score2 - score)<<" "<< std::abs((score2 - score)*100.0f/score)<<std::endl;
        std::cout<<score2 <<" "<<score <<std::endl;
        //std::cout<<score2 <<" "<<score <<std::endl;
}
#endif
    /*std::cout<<"---------END--------"<<std::endl;*/


    return score;
}

void NNUE::_resetCompleteEvalCondition() {
    _noIncrementalEval = false;
}
