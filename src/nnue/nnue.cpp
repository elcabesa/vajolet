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

//TODO remove std::set, std::map and aòò the very slow code
//TODO hardcode the nn 
//TODO move all the math in fixed point?

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
	_modelW.clear();
	_modelB.clear();

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
    _modelW.addLayer(std::make_unique<ParallelDenseLayer>(2, 40960, 256, _linear, biases0, weights0));
    _modelW.addLayer(std::make_unique<DenseLayer>(512,32, _relu, &bias1, &weight1));
    _modelW.addLayer(std::make_unique<DenseLayer>(32,32, _relu, &bias2, &weight2));
    _modelW.addLayer(std::make_unique<DenseLayer>(32, 1, _linear, &bias3, &weight3));

	_modelB.addLayer(std::make_unique<ParallelDenseLayer>(2, 40960, 256, _linear, biases0, weights0));
    _modelB.addLayer(std::make_unique<DenseLayer>(512,32, _relu, &bias1, &weight1));
    _modelB.addLayer(std::make_unique<DenseLayer>(32,32, _relu, &bias2, &weight2));
    _modelB.addLayer(std::make_unique<DenseLayer>(32, 1, _linear, &bias3, &weight3));


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

	_whiteW.clear();
    _blackW.clear();
    _whiteB.clear();
    _blackB.clear();
}

bool NNUE::load(std::string path) {

    std::cout<<"reload NNUE parameters"<<std::endl;    
	std::cout<<"deserialize"<<std::endl;

	std::ifstream nnFile;
	nnFile.open(path);
	if(nnFile.is_open()) {
		//todo, it's possibile to clone model?
		if(_modelW.deserialize(nnFile) 
			&& _modelB.deserialize(nnFile)
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
	//std::cout<<"inc: "<<incrementalCount<< " complete: "<<completeCount<<std::endl;
	
	if (whiteNoIncrementalEval || blackNoIncrementalEval) {
		whiteNoIncrementalEval = false;
		blackNoIncrementalEval = false;
		return _completeEval(pos);
	}
	return _incrementalEval(pos);
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

void NNUE::createBlackFeatures(const Position& pos, FeatureList& fl){
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
            fl.add(blackFeature(piece, pieceSq, bkSq));
		}
	}
}

void NNUE::createWhiteFeatures(const Position& pos, FeatureList& fl){
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
			fl.add(whiteFeature(piece, pieceSq, wkSq));
		}
	}
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

void NNUE::concatenateFeature(FeatureList f1, FeatureList f2, FeatureList& complete) {
	for(unsigned int i = 0; i < f1.size(); ++i) {
		complete.add(f1.get(i));
	}
	for(unsigned int i = 0; i < f2.size(); ++i) {
		complete.add(f2.get(i) + 40960);
	}
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
	//std::cout<<"remove piece "<<piece<<" square "<<sq<<std::endl;
	auto wf = whiteFeature(mapWhitePiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::whiteKing));
	auto bf = blackFeature(mapBlackPiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::blackKing));

	_whiteW.remove(wf);
	_blackW.remove(bf);
	_whiteB.remove(wf);
	_blackB.remove(bf);
}
void NNUE::addPiece(const Position& pos, bitboardIndex piece, tSquare sq) {
	//std::co_bFeaturesut<<"add piece "<<piece<<" square "<<sq<<std::endl;
	auto wf = whiteFeature(mapWhitePiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::whiteKing));
	auto bf = blackFeature(mapBlackPiece(piece), sq, pos.getSquareOfThePiece(bitboardIndex::blackKing));
	
	_whiteW.add(wf);
	_blackW.add(bf);
	_whiteB.add(wf);
	_blackB.add(bf);
}

Score NNUE::_completeEval(const Position& pos) {
	Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
	//++completeCount;
	//std::cout<<"no incremental eval"<<std::endl;
	// TODO if we move one king calculate everything from scratch
	// can we do it faster??
	_completeWhiteFeatureList.clear();
	_completeBlackFeatureList.clear();
	createWhiteFeatures(pos, _completeWhiteFeatureList);
	createBlackFeatures(pos, _completeBlackFeatureList);

	_whiteW.clear();
	_blackW.clear();
	_whiteB.clear();
	_blackB.clear();

	Score score;
	_completeFeatureList.clear();
	concatenateFeature(_completeWhiteFeatureList, _completeBlackFeatureList, _completeFeatureList);
	SparseInput spw(81920);
	for(unsigned int i = 0; i < _completeFeatureList.size(); ++i) {
		spw.set(_completeFeatureList.get(i), 1.0);
	}
	Score scoreW = _modelW.forwardPass(spw).get(0) * 10000.0;

	_completeFeatureList.clear();
	concatenateFeature(_completeBlackFeatureList, _completeWhiteFeatureList, _completeFeatureList);
	SparseInput spb(81920);
	for(unsigned int i = 0; i < _completeFeatureList.size(); ++i) {
		spb.set(_completeFeatureList.get(i), 1.0);
	}
	Score scoreB = _modelB.forwardPass(spb).get(0) * 10000.0;

	if(pos.isWhiteTurn()) {
		score = scoreW;
	} else {
		score = scoreB;
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

Score NNUE::_incrementalEval(const Position& pos) {
	const unsigned int CompleteEvalThreshold = 50;
	//++incrementalCount;
	Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
	Score score;
	if(pos.isWhiteTurn()) {
		if(_whiteW.size() + _blackW.size() > CompleteEvalThreshold) {return _completeEval(pos);}
		//std::cout<<"white"<<std::endl;
		SparseInput sp(81920);
		_whiteW.serialize(sp, 0);
    	_blackW.serialize(sp, 40960);
		//incrementalMaxSize = std::max(sp.getElementNumber(), incrementalMaxSize);
		//sp.print();
		score = _modelW.incrementalPass(sp).get(0)* 10000.0;
		_whiteW.clear();
    	_blackW.clear();
	} else {
		if(_whiteB.size() + _blackB.size() > CompleteEvalThreshold) {return _completeEval(pos);}
		//std::cout<<"black"<<std::endl;
		SparseInput sp(81920);
		_whiteB.serialize(sp, 40960);
    	_blackB.serialize(sp, 0);
		//incrementalMaxSize = std::max(sp.getElementNumber(), incrementalMaxSize);
		//sp.print();
		score = _modelB.incrementalPass(sp).get(0)* 10000.0;
		_whiteB.clear();
    	_blackB.clear();

	}

	//std::cout<<"incrementalMaxSize " << incrementalMaxSize << std::endl;
	
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


void DifferentialList::clear() {
	_addPos = 0;
	_removePos = 0;
}

void DifferentialList::serialize(SparseInput& s, unsigned int offset) {
	for(unsigned int i = 0; i < _addPos; ++i) {
		s.set(_addList[i] + offset, 1.0);
	}
	for(unsigned int i = 0; i < _removePos; ++i) {
		s.set(_removeList[i] + offset, -1.0);
	}
}

void DifferentialList::add(unsigned int f) {
	// search in remove
	for(unsigned int i = 0; i < _removePos; ++i) {
		if(_removeList[i] == f) {
			// found remove it
			for(unsigned int x = i; x < _removePos; ++x) {
				_removeList[x] = _removeList[x+1];
			}
			--_removePos;
			return;
		}
	}
	_addList[_addPos++] = f;

}
void DifferentialList::remove(unsigned int f) {
	// search in remove
	for(unsigned int i = 0; i < _addPos; ++i) {
		if(_addList[i] == f) {
			// found remove it
			for(unsigned int x = i; x < _addPos; ++x) {
				_addList[x] = _addList[x+1];
			}
			--_addPos;
			return;
		}
	}
	_removeList[_removePos++] = f;
}

unsigned int DifferentialList::size() {
	return _addPos + _removePos;
}

void FeatureList::clear() {
	_pos = 0;
}

void FeatureList::add(unsigned int f) {
	// search in remove
	_list[_pos++] = f;

}

unsigned int FeatureList::get(unsigned int index) {
	return _list[index];
}

unsigned int FeatureList::size() {
	return _pos;
}