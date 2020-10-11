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
	_sl.setDepth(6);
	_tt.setSize(64);
}


void FenSaver::save(Position& pos) {
	if (++_counter >= _decimation) {
		//std::cout<<"THREAD "<<_n<<" start search"<<std::endl;

		// do a low depth search to find a quiet position
		_sl.setDepth(4);
		//std::cout<<"start search"<<std::endl;
		//std::cout<<"fen "<<pos.getFen()<<std::endl;
		_src.getPosition() = pos;
		auto res = _src.manageNewSearch(*timeManagement::create(_sl, pos.getNextTurn()));
		
		//std::cout<<"follow pv"<<std::endl;
		for(unsigned int i = 0; i < res.PV.size(); ++i) {
			Move m = res.PV.getMove(i);
			if(m == Move::NOMOVE) {
				break;
			}
			//std::cout<<"move "<<UciOutput::displayUci(m, false)<<std::endl;
			pos.doMove(m);
			pos.nnue().clean();
		}
		//std::cout<<"RESULT "<<res.Res<<std::endl;
		//std::cout<<"check is drawn"<<std::endl;
		// exclude draw position
		if (pos.isDraw(false)) {
			return;
		}

		//std::cout<<"check eval"<<std::endl;
		Score eval = pos.eval<false>();
		if (eval == 0) {
			return;
		}
		//std::cout<<"eval "<<eval<<std::endl;

		if (std::abs(eval) > 200000) {
			return;
		}

		if (std::abs(res.Res) > 200000) {
			return;
		}

		_sl.setDepth(4);
		//std::cout<<"start search2"<<std::endl;
		_src.getPosition() = pos;
		auto res2 = _src.manageNewSearch(*timeManagement::create(_sl, pos.getNextTurn()));
		//std::cout<<"follow pv2"<<std::endl;
		for(unsigned int i = 0; i < res2.PV.size(); ++i) {
			Move m = res2.PV.getMove(i);
			if(m == Move::NOMOVE) {
				break;
			}
			//std::cout<<"move "<<UciOutput::displayUci(m, false)<<std::endl;
			pos.doMove(m);
			pos.nnue().clean();
		}

		//std::cout<<"RESULT "<<res2.Res<<std::endl;

		//std::cout<<"check is drawn"<<std::endl;
		// exclude draw position
		if (pos.isDraw(false)) {
			return;
		}
		
		//std::cout<<"check eval"<<std::endl;
		Score eval2 = pos.eval<false>();
		//std::cout<<"eval "<<eval2<<std::endl;
		if (eval2 == 0) {
			return;
		}

		if (std::abs(eval2) > 200000) {
			return;
		}
		//std::cout<<"THREAD "<<_n<<" done search"<<std::endl;
        if (std::abs(res2.Res) > 200000) {
			return;
		}

		//undo PV
		//std::cout<<"undo PV2"<<std::endl;
		for(unsigned int i = 0; i < res2.PV.size(); ++i) {
			pos.undoMove();
			pos.nnue().clean();
		}

		_totalError += std::pow((res2.Res - eval), 2.0) / 2.0;
		_counter = 0;

		++_logDecimationCnt;
		++_saved;

		if(_logDecimationCnt>=1000) {
				std::cout << "thread "<<_n<<" saved " << _saved << " FENs" <<std::endl;
				std::cout << "avg cost "<< _totalError/_saved<<std::endl;
		}
		
		//std::cout<<"save pos"<<std::endl;
		writeFeatures(pos);
		//std::cout<<"save res"<<std::endl;
		writeRes(res2.Res);
		_stream << std::endl;
		//_stream << pos.getFen()<< std::endl;
	}
}

void FenSaver::writeFeatures(Position& pos) {
	auto features = pos.nnue().createFeatures();
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
