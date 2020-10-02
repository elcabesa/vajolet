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

#include <iostream>

#include "nnue.h"
#include "parallelDenseLayer.h"
#include "position.h"
#include "sparse.h"

//TODO search multi thread... preallocate worker and use them, otherwise we call NNUE ctor for every search
//TODO movegen  calc & update feature only if nn is used
//TODO split init and load.... all position shall contain a NNUE? or one NNUE for each search.... command will call static load?
//TODO move all the math in fixed point?
//TODO incrememental update of NN
//TODO have 2 neural network one for white end one for black. it souhld be faster updating them

std::vector<double> NNUE::bias00;
std::vector<double> NNUE::bias01;
std::vector<double> NNUE::bias1;
std::vector<double> NNUE::bias2;
std::vector<double> NNUE::bias3;

std::vector<double> NNUE::weight00;
std::vector<double> NNUE::weight01;
std::vector<double> NNUE::weight1;
std::vector<double> NNUE::weight2;
std::vector<double> NNUE::weight3;

bool NNUE::_loaded = false;

NNUE::NNUE() {
	//std::cout<<"creating NNUE model"<<std::endl;
	_model.clear();

	bias00.resize(256, 0.0);
	bias01.resize(256, 0.0);
	bias1.resize(32, 0.0);
	bias2.resize(32, 0.0);
	bias3.resize(1, 0.0);
	
	weight00.resize(40960 * 256, 1.0);
	weight01.resize(40960 * 256, 1.0);
	weight1.resize(512 * 32, 1.0);
	weight2.resize(32 * 32, 1.0);
	weight3.resize(32 * 1, 1.0);

	std::vector<std::vector<double>*> biases0;
	biases0.push_back(&bias00);
	biases0.push_back(&bias01);
	std::vector<std::vector<double>*> weights0;
	weights0.push_back(&weight00);
	weights0.push_back(&weight01);
    _model.addLayer(std::make_unique<ParallelDenseLayer>(2, 40960, 256, _linear, biases0, weights0));
    _model.addLayer(std::make_unique<DenseLayer>(512,32, _relu, &bias1, &weight1));
    _model.addLayer(std::make_unique<DenseLayer>(32,32, _relu, &bias2, &weight2));
    _model.addLayer(std::make_unique<DenseLayer>(32, 1, _linear, &bias3, &weight3));

#ifdef CHECK_NNUE_FEATURE_EXTRACTION
	_m.clear();
	_m.addLayer(std::make_unique<ParallelDenseLayer>(2, 40960, 256, _linear, biases0, weights0));
    _m.addLayer(std::make_unique<DenseLayer>(512,32, _relu, &bias1, &weight1));
    _m.addLayer(std::make_unique<DenseLayer>(32,32, _relu, &bias2, &weight2));
    _m.addLayer(std::make_unique<DenseLayer>(32, 1, _linear, &bias3, &weight3));

#endif
    //std::cout<<"done"<<std::endl;

	whiteNoIncrementalEval = true;
    blackNoIncrementalEval = true;

	_whiteAdd.clear();
    _whiteRemove.clear();
    _blackAdd.clear();
    _blackRemove.clear();
}

bool NNUE::load(std::string path) {

    std::cout<<"reload NNUE parameters"<<std::endl;    
	std::cout<<"deserialize"<<std::endl;

	std::ifstream nnFile;
	nnFile.open(path);
	if(nnFile.is_open()) {
		//todo, it's possibile to clone model?
		if(_model.deserialize(nnFile)
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
			&& _m.deserialize(nnFile)
#endif
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

void NNUE::clear() {
	_loaded = false;
	//_model.clear();
}

Score NNUE::eval(const Position& pos) {
    Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
	
	if (whiteNoIncrementalEval || blackNoIncrementalEval) {
		// TODO if we move one king calculate everything from scratch
		// can we do it faster??
		_wFeatures = createWhiteFeatures(pos);
		_bFeatures = createBlackFeatures(pos);
		whiteNoIncrementalEval = false;
		blackNoIncrementalEval = false;
		_whiteAdd.clear();
		_whiteRemove.clear();
		_blackAdd.clear();
		_blackRemove.clear();
	}

	auto f2 = concatenateFeature(pos.isWhiteTurn(), _wFeatures, _bFeatures);
	SparseInput sp(81920, std::vector<unsigned int>(f2.begin(), f2.end()));
	Score score;
	if(pos.isWhiteTurn()) {
		// todo help
		score = _model.forwardPass(sp).get(0)* 10000.0;
	} else {
		score = _model.forwardPass(sp).get(0)* 10000.0;
	}
	
#ifdef CHECK_NNUE_FEATURE_EXTRACTION
	Score score2;
	auto f1 = createFeatures(pos);
	SparseInput sp2(81920, std::vector<unsigned int>(f1.begin(), f1.end()));
	score2 = _m.forwardPass(sp2).get(0)* 10000.0;
	if(score2 != score) {
		std::cout<<"AHHHHHHHHHHHHHHHHHHHHH"<<std::endl;
	}
#endif


    score = std::min(highSat,score);
	score = std::max(lowSat,score);
    return score;
    
}

std::set<unsigned int> NNUE::createFeatures(const Position& pos){
    std::set<unsigned int> features;
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
	
	bool whiteTurn = pos.isWhiteTurn();
	bool blackTurn = pos.isBlackTurn();
	
	tSquare wkSq = pos.getSquareOfThePiece(bitboardIndex::whiteKing);
	for(unsigned int piece = 0; piece < 10; ++piece) {
    
		bitMap b = pos.getBitmap(whitePow[piece]);
		while(b)
		{
			tSquare pieceSq = iterateBit(b);
			features.insert(calcWhiteFeature(whiteTurn, piece, pieceSq, wkSq));
		}
	}
	
	tSquare bkSq = pos.getSquareOfThePiece(bitboardIndex::blackKing);
	for(unsigned int piece = 0; piece < 10; ++piece) {
		
		bitMap b = pos.getBitmap(blackPow[piece]);
		while(b)
		{
			tSquare pieceSq = iterateBit(b);
            features.insert(calcBlackFeature(blackTurn, piece, pieceSq, bkSq));
		}
	}
	 
	return features;
}

std::set<unsigned int> NNUE::createBlackFeatures(const Position& pos){
    std::set<unsigned int> features;

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
	
	tSquare bkSq = pos.getSquareOfThePiece(bitboardIndex::blackKing);
	for(unsigned int piece = 0; piece < 10; ++piece) {
		
		bitMap b = pos.getBitmap(blackPow[piece]);
		while(b)
		{
			tSquare pieceSq = iterateBit(b);
            features.insert(blackFeature(piece, pieceSq, bkSq));
		}
	}
	 
	return features;
}

std::set<unsigned int> NNUE::createWhiteFeatures(const Position& pos){
    std::set<unsigned int> features;
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
	
	tSquare wkSq = pos.getSquareOfThePiece(bitboardIndex::whiteKing);
	for(unsigned int piece = 0; piece < 10; ++piece) {
    
		bitMap b = pos.getBitmap(whitePow[piece]);
		while(b)
		{
			tSquare pieceSq = iterateBit(b);
			features.insert(whiteFeature(piece, pieceSq, wkSq));
		}
	}
	 
	return features;
}

bool NNUE::loaded() {
    return _loaded;
}

unsigned int NNUE::calcWhiteFeature(bool myTurn, unsigned int  piece, tSquare pSquare, tSquare ksq) {
    return turnOffset(myTurn) + whiteFeature(piece, pSquare, ksq);
}

unsigned int NNUE::calcBlackFeature(bool myTurn, unsigned int  piece, tSquare pSquare, tSquare ksq) {
    return turnOffset(myTurn) + blackFeature(piece, pSquare, ksq);
}

unsigned int NNUE::whiteFeature(unsigned int  piece, tSquare pSquare, tSquare ksq) {
	unsigned int f = piece + (10 * pSquare) + (640 * ksq);
    return f;
}

unsigned int NNUE::blackFeature(unsigned int  piece, tSquare pSquare, tSquare ksq) {
    unsigned int f = piece + (10 * (pSquare^56)) + (640 * (ksq^56));
    return f;
}

unsigned int NNUE::turnOffset(bool myTurn) {
	return myTurn ? 0 : 40960;
}

std::set<unsigned int> NNUE::concatenateFeature(bool whiteTurn, std::set<unsigned int> w, std::set<unsigned int> b) {
	std::set<unsigned int> features;
	if (whiteTurn) {
		features = w;
		for(auto & f: b) {
			features.insert(f + 40960);
		}
	} else {
		features = b;
		for(auto & f: w) {
			features.insert(f + 40960);
		}
	}

	return features;
}

unsigned int NNUE::mapWhitePiece(const bitboardIndex piece) {
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
unsigned int NNUE::mapBlackPiece(const bitboardIndex piece) {
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

void NNUE::removePiece(const Position& pos, bitboardIndex piece, tSquare sq) {
	auto wf = whiteFeature(mapWhitePiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::whiteKing));
	auto bf = blackFeature(mapBlackPiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::blackKing));
	_wFeatures.erase(wf);
	_bFeatures.erase(bf);

	if(auto it = _whiteAdd.find(wf); it != _whiteAdd.end()) {
		_whiteAdd.erase(it);
	} else {
    	_whiteRemove.insert(wf);
	}
	if(auto it = _blackAdd.find(bf); it != _blackAdd.end()) {
		_blackAdd.erase(it);
	} else {
    	_blackRemove.insert(bf);
	}
}
void NNUE::addPiece(const Position& pos, bitboardIndex piece, tSquare sq) {
	auto wf = whiteFeature(mapWhitePiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::whiteKing));
	auto bf = blackFeature(mapBlackPiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::blackKing));
	_wFeatures.insert(wf);
	_bFeatures.insert(bf);

	if(auto it = _whiteRemove.find(wf); it != _whiteRemove.end()) {
		_whiteRemove.erase(it);
	} else {
    	_whiteAdd.insert(wf);
	}
	if(auto it = _blackRemove.find(bf); it != _blackRemove.end()) {
		_blackRemove.erase(it);
	} else {
    	_blackAdd.insert(bf);
	}
}
