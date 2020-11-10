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
	_sl.setInfiniteSearch();
	_sl.setDepth(6);
	_tt.setSize(64);
}


void FenSaver::save(Position& pos) {
	unsigned int searchDepth = 7;
	if (++_counter >= _decimation) {
		_counter = 0;
		

		++_logDecimationCnt;
		++_saved;

		if(_logDecimationCnt>=1000) {
				std::cout << "thread "<<_n<<" saved " << _saved << " FENs" <<std::endl;
				//std::cout << "avg cost "<< _totalError/_saved<<std::endl;
				//std::cout << "highDiff "<< _highDiffCnt<<"/"<<_totalCnt<<std::endl;
		}
		
		//std::cout<<"save pos"<<std::endl;
		writeFeatures(pos);
		//std::cout<<"save res"<<std::endl;
		writeRes(pos.eval<false>());
		_stream << std::endl;
		//_stream << pos.getFen()<< std::endl;
	}
}

void FenSaver::writeFeatures(Position& pos) {
	auto features = pos.nnue()->features();
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
