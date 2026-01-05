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
#include "model.h"

INCBIN(NnueInternalFile, "../../NN/nnue.par");

constexpr static outType _scaleFl = scaleFL; //Q12
constexpr static outType _scaleSl = scaleSL; //Q12


bool NNUE::_loaded = false;

NNUE::NNUE(const Position& pos):_model(std::make_shared<Model>(_scaleFl, _scaleSl)),
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
_modelCheck(std::make_shared<Model>(_scaleFl, _scaleSl)),
#endif
_pos(pos) {
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
        if(_model->deserialize(nnFile)){
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
            if(_model->deserialize(nnFile)
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

void NNUE::printStats() {
    _model->printStats();
}

Score NNUE::eval() {
    Score s;
    if (incrementalEvalDisabled()) {
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


    FeatureList fl2;

    for(unsigned int i = 0; i< fl.size(); ++i) {
        unsigned int  idx = fl.get(i);
        if(idx < 384) {idx +=384;}
        else {idx -=384;}
        idx = idx ^ 0b111000;
        fl2.add(idx);
    }

    auto s  = _model->forwardPass(fl, fl2, whitePow); //Q24

    Score score = s * (evalScale / (_scaleFl*_scaleSl) );
    score = std::min(highSat, score);
    score = std::max(lowSat, score);
    return score;
}

std::set<unsigned int> NNUE::features() {
    std::set<unsigned int> features;
    //std::set<unsigned int> features2;

    {
        FeatureList fl;
        _createFeatures(fl, (_pos.isBlackTurn()? blackPow: whitePow));
        for(unsigned int i = 0; i< fl.size(); ++i) {
            features.insert(fl.get(i));
        }
    }

    /*{
        FeatureList fl;
        _createFeatures(fl, (_pos.isBlackTurn()? whitePow : blackPow));
        for(unsigned int i = 0; i< fl.size(); ++i) {
            features2.insert(fl.get(i));
        }
    }
    std::cout<<"features A:";
    for(auto f: features) std::cout<<f<<",";
    std::cout<<std::endl;
    std::cout<<"features b:";
    for(auto f: features2) std::cout<<f<<",";
    std::cout<<std::endl;*/

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

bool NNUE::incrementalEvalDisabled() const {
    return _noIncrementalEval;
}

void NNUE::removePiece(bitboardIndex piece, tSquare sq) {

    auto f = _feature(_mapPiece(piece, whitePow), sq, whitePow);
    _diffFeatureListW.remove(f);

    f = _feature(_mapPiece(piece, blackPow), sq, blackPow);
    _diffFeatureListB.remove(f);

    if (_diffFeatureListW.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
    if (_diffFeatureListB.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
}

void NNUE::addPiece(bitboardIndex piece, tSquare sq) {
    auto f = _feature(_mapPiece(piece, whitePow), sq, whitePow);
    _diffFeatureListW.add(f);

    f = _feature(_mapPiece(piece, blackPow), sq, blackPow);
    _diffFeatureListB.add(f);

    if (_diffFeatureListW.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
    if (_diffFeatureListB.size() >= _completeEvalThreshold) { _disableIncrementalEval(); }
}

void NNUE::_createFeatures(FeatureList& fl, perspective p){

    bitboardIndex bWhitePow[12] = {
        whitePawns,
        whiteKnights,
        whiteBishops,
        whiteRooks,
        whiteQueens,
        whiteKing,
        blackPawns,
        blackKnights,
        blackBishops,
        blackRooks,
        blackQueens,
        blackKing,
    };
    bitboardIndex bBlackPow[12] = {
        blackPawns,
        blackKnights,
        blackBishops,
        blackRooks,
        blackQueens,
        blackKing,
        whitePawns,
        whiteKnights,
        whiteBishops,
        whiteRooks,
        whiteQueens,
        whiteKing
    };

    for(unsigned int piece = 0; piece < 12; ++piece) {
        bitMap b = _pos.getBitmap(p==whitePow? bWhitePow[piece] : bBlackPow[piece] );
        while(b)
        {
            tSquare pieceSq = iterateBit(b);
            fl.add(_feature(piece, pieceSq, p));
        }
    }
}

unsigned int NNUE::_feature(unsigned int  piece, tSquare pSquare, perspective p) {
    if(p == whitePow) {
        return (piece * 64) + (pSquare);
    } else {
        return (piece * 64) + (pSquare ^ 0b111000);
    }
}

tSquare NNUE::_getSquareFromFeature(unsigned int f) {return tSquare(f&63);} // TODO serve fare POW??

bitboardIndex NNUE::_getPieceFromFeature(unsigned int f) { // TODO SERVE FARE POW?
    bitboardIndex whitePow[12] = {
        whitePawns,
        whiteKnights,
        whiteBishops,
        whiteRooks,
        whiteQueens,
        whiteKing,
        blackPawns,
        blackKnights,
        blackBishops,
        blackRooks,
        blackQueens,
        blackKing
    };
    return whitePow[f>>6];}


unsigned int NNUE::_mapPiece(const bitboardIndex piece, perspective p) {
    if(p == whitePow) {
        unsigned int n[] = {
            0, //occupiedSquares,			//0		00000000
            5, //whiteKing,					//1		00000001
            4, //whiteQueens,				//2		00000010
            3, //whiteRooks,				//3		00000011
            2, //whiteBishops,				//4		00000100
            1, //whiteKnights,				//5		00000101
            0, //whitePawns,				//6		00000110
            0, //whitePieces,				//7		00000111

            0, //separationBitmap,			//8
            11, //blackKing,				//9		00001001
            10, //blackQueens,				//10	00001010
            9, //blackRooks,				//11	00001011
            8, //blackBishops,				//12	00001100
            7,//blackKnights,				//13	00001101
            6,//blackPawns,					//14	00001110
            0  //blackPieces,				//15	00001111
        };
        return n[piece];
    } else {
        unsigned int n[] = {
            0, //separationBitmap,			//8
            11, //blackKing,				//9		00001001
            10, //blackQueens,				//10	00001010
            9, //blackRooks,				//11	00001011
            8, //blackBishops,				//12	00001100
            7,//blackKnights,				//13	00001101
            6,//blackPawns,					//14	00001110
            0, //blackPieces,				//15	00001111

            0, //occupiedSquares,			//0		00000000
            5, //whiteKing,					//1		00000001
            4, //whiteQueens,				//2		00000010
            3, //whiteRooks,				//3		00000011
            2, //whiteBishops,				//4		00000100
            1, //whiteKnights,				//5		00000101
            0, //whitePawns,				//6		00000110
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
    _createFeatures(_completeFeatureListW, whitePow);
    _createFeatures(_completeFeatureListB, blackPow);
    auto s  = _model->forwardPass(_completeFeatureListW, _completeFeatureListB, _pos.isBlackTurn() ? blackPow : whitePow);

    Score score = s * (evalScale / (_scaleFl*_scaleSl));

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

    Score score = _model->incrementalPass(_diffFeatureListW, _diffFeatureListB, _pos.isBlackTurn() ? blackPow : whitePow) * (evalScale / (_scaleFl*_scaleSl));
    _diffFeatureListB.clear();
    _diffFeatureListW.clear();


    score = std::min(highSat,score);
    score = std::max(lowSat,score);

#ifdef CHECK_NNUE_FEATURE_EXTRACTION

    FeatureList flw;
    FeatureList flb;
    flw.clear();
    flb.clear();
    _createFeatures(flw, whitePow);
    _createFeatures(flb, blackPow);
    Score score2 = _modelCheck->forwardPass(flw, flb, _pos.isBlackTurn() ? blackPow : whitePow) * (evalScale / (_scaleFl*_scaleSl));
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
