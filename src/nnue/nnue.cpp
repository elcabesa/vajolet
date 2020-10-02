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


NNUE::NNUE(): _loaded{false} {}

void NNUE::init() {
    
    std::cout<<"creating NNUE model"<<std::endl;
    _model.addLayer(std::make_unique<ParallelDenseLayer>(2, 40960, 256, ActivationFactory::create(ActivationFactory::type::linear)));
    _model.addLayer(std::make_unique<DenseLayer>(512,32, ActivationFactory::create(ActivationFactory::type::relu)));
    _model.addLayer(std::make_unique<DenseLayer>(32,32, ActivationFactory::create(ActivationFactory::type::relu)));
    _model.addLayer(std::make_unique<DenseLayer>(32, 1, ActivationFactory::create(ActivationFactory::type::linear)));
    std::cout<<"done"<<std::endl;
    
    std::cout<<"reload NNUE parameters"<<std::endl;
    {
        std::cout<<"deserialize"<<std::endl;
        std::ifstream nnFile;
        nnFile.open ("nnue.par");
        if(_model.deserialize(nnFile)){
            _loaded = true;
             std::cout<<"done"<<std::endl;
        }else {
             std::cout<<"FAIL"<<std::endl;
        }
        nnFile.close();
    }
    std::cout<<"done"<<std::endl;
    
}

Score NNUE::eval(const Position& pos) {
    Score lowSat = -SCORE_INFINITE;
    Score highSat = SCORE_INFINITE;
    auto f = createFeatures(pos);
    SparseInput sp(81920, f);
    Score score = _model.forwardPass(sp).get(0)* 10000.0;
    score = std::min(highSat,score);
	score = std::max(lowSat,score);
    return score;
    
}

std::vector<unsigned int> NNUE::createFeatures(const Position& pos) const {
    std::vector<unsigned int> features;
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
	

	tSquare wksq = pos.getSquareOfThePiece(bitboardIndex::whiteKing);
	tSquare pieceSq;
	unsigned int piece;
	for(piece = 0; piece < 10; ++piece) {
    
		bitMap b = pos.getBitmap(whitePow[piece]);
		while(b)
		{
			pieceSq = iterateBit(b);
			unsigned int feature = (whiteTurn? 0 : 40960 ) + piece + (10 * pieceSq) + (640 * wksq);
			features.push_back(feature);
		}
	}
	
	tSquare bksq = pos.getSquareOfThePiece(bitboardIndex::blackKing);
	for(piece = 0; piece < 10; ++piece) {
		
		bitMap b = pos.getBitmap(blackPow[piece]);
		while(b)
		{
			pieceSq = iterateBit(b);
			unsigned int feature = (whiteTurn? 40960 : 0 ) + piece + (10 * (pieceSq^56)) + (640 * (bksq^56));
			features.push_back(feature);
		}
	}
	return features;
}

bool NNUE::loaded() const {
    return _loaded;
}
