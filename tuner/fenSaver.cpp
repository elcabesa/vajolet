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

#include "fenSaver.h"
#include "position.h"
#include "searchResult.h"
#include "timeManagement.h"
#include "transposition.h"


FenSaver::FenSaver(unsigned int decimation): _decimation(decimation), _src(_st, _sl, _tt, UciOutput::create(UciOutput::type::mute)){
	_stream.open("fen.csv");
	_sl.setDepth(8);
	_tt.setSize(64);
}


void FenSaver::save(const Position& pos) {
	if (++_counter >= _decimation) {
        _src.getPosition() = pos;
		auto res = _src.manageNewSearch(*timeManagement::create(_sl, pos.getNextTurn())).Res;
        auto eval = pos.eval<false>();
        if (std::abs(res)< SCORE_KNOWN_WIN && std::abs(eval)< SCORE_KNOWN_WIN) {
		    _counter = 0;
			++_logDecimationCnt;
			++_saved;
			if(_logDecimationCnt>=1000) {
					std::cout << "saved " << _saved << " FENs" <<std::endl;
			}
		    
            writeFeatures(pos);
			writeRes(res);
			_stream << std::endl;
		    //_stream << pos.getFen() << "," << eval << "," << res << std::endl;
        }
		
	}
}

void FenSaver::writeFeatures(const Position& pos) {
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
	
	_stream <<'{';
	
	bool whiteTurn = pos.isWhiteTurn();
	
	//std::cout<<pos.getFen()<<std::endl;
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
            featuresIndex.insert(feature);
			//std::cout<<"FEATURE "<<feature<<std::endl;
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
            featuresIndex.insert(feature);
			//std::cout<<"FEATURE "<<feature<<std::endl;
		}
	}
	
	
    for(unsigned int i =0; i < features.size(); ++i) {
        _stream<<features[i];
        if(i < features.size() -1) {_stream <<',';}
    }
    _stream <<'}';
    if(_logDecimationCnt>=1000) {
		_logDecimationCnt = 0;
		std::cout<<"features "<<featuresIndex.size() <<"/81920 ("<< featuresIndex.size() *100.0/81920<<"%)"<<std::endl; 
	}

	//exit(0);
}

void FenSaver::writeRes(Score res) {
	_stream <<'{'<<res<<'}';
	
}
