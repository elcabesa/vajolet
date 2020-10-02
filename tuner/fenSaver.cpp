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
#include <cmath>
#include "fenSaver.h"
#include "position.h"
#include "searchResult.h"
#include "timeManagement.h"
#include "transposition.h"


FenSaver::FenSaver(unsigned int decimation, unsigned int n): _decimation(decimation), _src(_st, _sl, _tt, UciOutput::create(UciOutput::type::mute)),_n(n){
	_stream.open("fen"+ std::to_string(n) + ".csv");
	_sl.setDepth(4);
	_tt.setSize(64);
}


void FenSaver::save(const Position& pos) {
	if (++_counter >= _decimation) {
        _src.getPosition() = pos;
		//std::cout<<"THREAD "<<_n<<" start search"<<std::endl;
		auto res = _src.manageNewSearch(*timeManagement::create(_sl, pos.getNextTurn())).Res;
		//std::cout<<"THREAD "<<_n<<" start Eval"<<std::endl;
        auto eval = pos.eval<false>();
		//std::cout<<"THREAD "<<_n<<" done search"<<std::endl;
        if (std::abs(res)< 200000) {
			_totalError += std::pow((res - eval), 2.0) / 2.0;
		    _counter = 0;
			++_logDecimationCnt;
			++_saved;
			if(_logDecimationCnt>=1000) {
					std::cout << "thread "<<_n<<" saved " << _saved << " FENs" <<std::endl;
					std::cout << "avg cost "<< _totalError/_saved<<std::endl;
			}
		    
            writeFeatures(pos);
			writeRes(res);
			_stream << std::endl;
		    //_stream << pos.getFen()<< std::endl;
        }
		
	}
}

void FenSaver::writeFeatures(const Position& pos) {
	std::set<unsigned int> features;
	bitboardIndex whitePow[10] = {
		whiteQueens, // 0
		whiteRooks,  // 1
		whiteBishops,// 2
		whiteKnights,// 3
		whitePawns,  // 4
		blackQueens, // 5
		blackRooks,  // 6
		blackBishops,// 7
		blackKnights,// 8
		blackPawns   // 9
	};
	
	bitboardIndex blackPow[10] = {
		blackQueens, // 0
		blackRooks,  // 1
		blackBishops,// 2
		blackKnights,// 3
		blackPawns,  // 4
		whiteQueens, // 5
		whiteRooks,  // 6
		whiteBishops,// 7
		whiteKnights,// 8
		whitePawns   // 9
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
			features.insert(feature);
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
			features.insert(feature);
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
		std::cout<<"thread "<<_n<<"features "<<featuresIndex.size() <<"/81920 ("<< featuresIndex.size() *100.0/81920<<"%)"<<std::endl; 
	}

	//exit(0);
}

void FenSaver::writeRes(Score res) {
	_stream <<'{'<<res<<'}';
	
}
