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

#include "linear.h"
#include "nnue.h"
#include "parallelDenseLayer.h"
#include "position.h"
#include "relu.h"
#include "sparse.h"

//TODO multithreaded code, a single set of weights and bias shall stored outside the model
//TODO move all the math in fixed point?
//TODO incrememental update of NN
//TODO have 2 neural network one for white end one for black. it souhld be faster updating them

NNUE::NNUE(): _loaded{false} {}

bool NNUE::init(std::string path) {
    
    std::cout<<"creating NNUE model"<<std::endl;
    _model.addLayer(std::make_unique<ParallelDenseLayer>(2, 40960, 256, ActivationFactory::create(ActivationFactory::type::linear)));
    _model.addLayer(std::make_unique<DenseLayer>(512,32, ActivationFactory::create(ActivationFactory::type::relu)));
    _model.addLayer(std::make_unique<DenseLayer>(32,32, ActivationFactory::create(ActivationFactory::type::relu)));
    _model.addLayer(std::make_unique<DenseLayer>(32, 1, ActivationFactory::create(ActivationFactory::type::linear)));
    std::cout<<"done"<<std::endl;
    
    std::cout<<"reload NNUE parameters"<<std::endl;
    
	std::cout<<"deserialize"<<std::endl;
	std::ifstream nnFile;
	nnFile.open(path);
	if(nnFile.is_open()) {
		if(_model.deserialize(nnFile)){
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
	_model.clear();
}

Score NNUE::eval(const Position& pos) {
    Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
    //auto f1 = createFeatures(pos);
	auto f2 = pos.getFeatures();
	/*if(f1 != f2) {
		std::cout<<"AHHHHHHHHHHHHHHHHHHh"<<std::endl;
	}*/
    SparseInput sp(81920, std::vector<unsigned int>(f2.begin(), f2.end()));
    Score score = _model.forwardPass(sp).get(0)* 10000.0;
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

bool NNUE::loaded() const {
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
	//std::cout<<f<<std::endl;
    return f;
}

unsigned int NNUE::blackFeature(unsigned int  piece, tSquare pSquare, tSquare ksq) {
    unsigned int f = piece + (10 * (pSquare^56)) + (640 * (ksq^56));
	//std::cout<<f<<std::endl;
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