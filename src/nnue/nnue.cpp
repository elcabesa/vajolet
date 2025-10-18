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
    Score s;
    if (_incrementalEvalDisabled()) {
        _resetCompleteEvalCondition();
        s =  _completeEval();
    } else {
        s = _incrementalEval();
    }
    return s;

}

Score NNUE::eval(FeatureList fl) {
    Score lowSat = -SCORE_KNOWN_WIN;
    Score highSat = SCORE_KNOWN_WIN;


    auto s  = _model.forwardPass(fl, fl, Model::whitePow); //Q24

    Score score = s * (50000.0f / (_scale*_scale) );
    score = std::min(highSat, score);
    score = std::max(lowSat, score);
    return score;
}

std::set<unsigned int> NNUE::features() {
    std::set<unsigned int> features;
    FeatureList fl;
    _createFeatures(fl, (_pos.isBlackTurn()? Model::blackPow: Model::whitePow));

    for(unsigned int i = 0; i< fl.size(); ++i) {
        features.insert(fl.get(i));
    }

    return features;
}

void NNUE::clean() {
    _disableIncrementalEval();
    _diffFeatureListW.clear();
    _diffFeatureListB.clear();
    _completeFeatureListW.clear();
    _completeFeatureListB.clear();
}

void NNUE::_disableIncrementalEval() {
    _noIncrementalEval = true;
}

bool NNUE::_incrementalEvalDisabled() const {
    return _noIncrementalEval;
}

void NNUE::removePiece(bitboardIndex piece, tSquare sq) {

    auto f = _feature(_mapPiece(piece, Model::whitePow), sq, Model::whitePow);
    _diffFeatureListW.remove(f);

    f = _feature(_mapPiece(piece, Model::blackPow), sq, Model::blackPow);
    _diffFeatureListB.remove(f);

    if (_diffFeatureListW.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
    if (_diffFeatureListB.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
}

void NNUE::addPiece(bitboardIndex piece, tSquare sq) {
    auto f = _feature(_mapPiece(piece, Model::whitePow), sq, Model::whitePow);
    _diffFeatureListW.add(f);

    f = _feature(_mapPiece(piece, Model::blackPow), sq, Model::blackPow);
    _diffFeatureListB.add(f);

    if (_diffFeatureListW.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
    if (_diffFeatureListB.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
}

void NNUE::_createFeatures(FeatureList& fl, Model::perspective p){

    bitboardIndex bWhitePow[12] = {
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
    bitboardIndex bBlackPow[12] = {
        blackKing,
        blackQueens,
        blackRooks,
        blackBishops,
        blackKnights,
        blackPawns,
        whiteKing,
        whiteQueens,
        whiteRooks,
        whiteBishops,
        whiteKnights,
        whitePawns
    };

    for(unsigned int piece = 0; piece < 12; ++piece) {
        bitMap b = _pos.getBitmap(p==Model::whitePow? bWhitePow[piece] : bBlackPow[piece] );
        while(b)
        {
            tSquare pieceSq = iterateBit(b);
            fl.add(_feature(piece, pieceSq, p));
        }
    }
}

unsigned int NNUE::_feature(unsigned int  piece, tSquare pSquare, Model::perspective p) {
    if(p == Model::whitePow) {
        return (piece * 64) + (pSquare);
    } else {
        return (piece * 64) + (pSquare ^ 0b111000);
    }
}

tSquare NNUE::_getSquareFromFeature(unsigned int f) {return tSquare(f&63);} // TODO serve fare POW??

bitboardIndex NNUE::_getPieceFromFeature(unsigned int f) { // TODO SERVE FARE POW?
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


unsigned int NNUE::_mapPiece(const bitboardIndex piece, Model::perspective p) {
    if(p == Model::whitePow) {
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
            10,//blackKnights,				//13	00001101
            11,//blackPawns,				//14	00001110
            0  //blackPieces,				//15	00001111
        };
        return n[piece];
    } else {
        unsigned int n[] = {
            0, //separationBitmap,			//8
            6, //blackKing,					//9		00001001
            7, //blackQueens,				//10	00001010
            8, //blackRooks,				//11	00001011
            9, //blackBishops,				//12	00001100
            10,//blackKnights,				//13	00001101
            11,//blackPawns,				//14	00001110
            0, //blackPieces,				//15	00001111

            0, //occupiedSquares,			//0		00000000
            0, //whiteKing,					//1		00000001
            1, //whiteQueens,				//2		00000010
            2, //whiteRooks,				//3		00000011
            3, //whiteBishops,				//4		00000100
            4, //whiteKnights,				//5		00000101
            5, //whitePawns,				//6		00000110
            0  //whitePieces,				//7		00000111
        };
        return n[piece];
    }
}

Score NNUE::_completeEval() {
    //std::cout<< "complete eval"<<std::endl;

    Score lowSat = -SCORE_KNOWN_WIN;
    Score highSat = SCORE_KNOWN_WIN;

    _diffFeatureListW.clear();// TODO remove?? da verificare se è più veloce o meno, non dovrebbe cambioare il numero di nodi ma solo nps
    _diffFeatureListB.clear();// TODO remove?? da verificare se è più veloce o meno, non dovrebbe cambioare il numero di nodi ma solo nps

    _completeFeatureListW.clear();
    _completeFeatureListB.clear();
    _createFeatures(_completeFeatureListW, Model::whitePow);
    _createFeatures(_completeFeatureListB, Model::blackPow);
    auto s  = _model.forwardPass(_completeFeatureListW, _completeFeatureListB, _pos.isBlackTurn() ? Model::blackPow : Model::whitePow);

    Score score = s * (50000.0f / (_scale*_scale));

    score = std::min(highSat, score);
    score = std::max(lowSat, score);
    return score;
}

Score NNUE::_incrementalEval() {
    //++incrementalCount;
    Score lowSat = -SCORE_KNOWN_WIN;
    Score highSat = SCORE_KNOWN_WIN;
    if(_diffFeatureListB.size() >= _completeEvalThreshold) {return _completeEval();}
    if(_diffFeatureListW.size() >= _completeEvalThreshold) {return _completeEval();}

    Score score = _model.incrementalPass(_diffFeatureListW, _diffFeatureListB, _pos.isBlackTurn() ? Model::blackPow : Model::whitePow) * (50000.0f / (_scale*_scale));
    _diffFeatureListB.clear();
    _diffFeatureListW.clear();


    score = std::min(highSat,score);
    score = std::max(lowSat,score);

#ifdef CHECK_NNUE_FEATURE_EXTRACTION

    FeatureList flw;
    FeatureList flb;
    flw.clear();
    flb.clear();
    _createFeatures(flw, Model::whitePow);
    _createFeatures(flb, Model::blackPow);
    Score score2 = _modelCheck.forwardPass(flw, flb, _pos.isBlackTurn() ? Model::blackPow : Model::whitePow) * (50000.0f / (_scale*_scale));
    score2 = std::min(highSat, score2);
    score2 = std::max(lowSat, score2);

    if(std::abs(score2 - score) > 3.0) {
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
