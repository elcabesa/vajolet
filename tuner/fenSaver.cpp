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
#include "nnue.h"
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
	auto features = NNUE::createFeatures(pos);
	_stream <<'{';

	unsigned int i = 0;
	for(auto& f: features) {
		_stream<<f;
		if(i++ < features.size() -1) {_stream <<',';}
		featuresIndex.insert(f);
	}
    _stream <<'}';

	if(_logDecimationCnt>=1000) {
		_logDecimationCnt = 0;
		std::cout<<"thread "<<_n<<"features "<<featuresIndex.size() <<"/81920 ("<< featuresIndex.size() *100.0/81920<<"%)"<<std::endl; 
	}
}

void FenSaver::writeRes(Score res) {
	_stream <<'{'<<res<<'}';
	
}
