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
#include "vajo_io.h"


FenSaver::FenSaver(unsigned int decimation, unsigned int n): _decimation(decimation), _src(_st, _sl, _tt, UciOutput::create(UciOutput::type::mute)),_pos(Position::nnueConfig::on, Position::pawnHash::off),_n(n){
	_stream.open("fen"+ std::to_string(n) + ".csv");
	_sl.setInfiniteSearch();
	_sl.setDepth(6);
	_tt.setSize(64);
}

FenSaver::QsearchRes FenSaver::_getQuiescentPosFeatures(const Position& p) {
	QsearchRes res;

	_pos = p;
	unsigned int pvLen = 0;
	_sl.setDepth(0);
	_src.getPosition() = _pos;
	auto srcRes = _src.manageNewSearch(*timeManagement::create(_sl, p.getNextTurn()));
	for(unsigned int i = 0; i < srcRes.PV.size(); ++i) {
		Move m = srcRes.PV.getMove(i);
		if(m == Move::NOMOVE) {
			/*std::cout<<res.PV.size()<<" "<<i<<std::endl;
			std::cout<<"ERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRORE1"<<std::endl;*/
			break;
		}
		++pvLen;
		_pos.doMove(m);
	}
	;
	res.fen = _pos.getFen();
	res.features = _pos.nnue()->features();
	res.invertRes = ((pvLen % 2) == 1); // odd pv
	res.res = _pos.eval<false>();

	return res;
}

Score FenSaver::_getSearchRes(const Position& p) {
	unsigned int searchDepth = 4;

	_sl.setDepth(searchDepth);
	_src.getPosition() = p;
	auto res = _src.manageNewSearch(*timeManagement::create(_sl, p.getNextTurn()));
	return res.Res;
}


void FenSaver::save(Position& pos) {
	if (++_counter >= _decimation) {

		if (pos.isDraw(false)) {
			return;
		}

		Score eval = pos.eval<false>();
		if (std::abs(eval) > 200000) {
			return;
		}

		auto qres = _getQuiescentPosFeatures(pos);
		
		if (std::abs(qres.res) > 200000) {
			return;
		}
		Score res = _getSearchRes(_pos);

		if (std::abs(res) > 200000) {
			return;
		}

		++_totalCnt;

		if(std::abs(res - qres.res) > 1e4) {
			++_highDiffCnt;
			//sync_cout<<pos.getFen()<<", "<<qres.fen<<", "<<res<<", "<<qres.res<<sync_endl;
			return;
		}
		/*if (res == qres.res) {
			sync_cout<<pos.getFen()<<", "<<qres.fen<<", "<<res<<", "<<qres.res<<sync_endl;
		}*/

		_totalError += std::pow((res - qres.res), 2.0);
		_counter = 0;
		
		++_logDecimationCnt;
		++_saved;

		if(_logDecimationCnt>=1000) {
				std::cout << "thread "<<_n<<" saved " << _saved << " FENs" <<std::endl;
				std::cout << "avg cost "<< _totalError/_saved<<std::endl;
				std::cout << "highDiff "<< _highDiffCnt<<"/"<<_totalCnt<<std::endl;
		}
		
		writeFen(qres);
		writeRes(/*qres.invertRes ? -res : */res);
		_stream << std::endl;
	}
}

void FenSaver::writeFen(QsearchRes& res) {
	_stream <<'{';
	_stream<<res.fen;
	for(auto& f: res.features) {
		featuresIndex.insert(f);
	}
    _stream <<'}';

	if(_logDecimationCnt>=1000) {
		_logDecimationCnt = 0;
		std::cout<<"thread "<<_n<<" features "<<featuresIndex.size() <<"/81920 ("<< featuresIndex.size() *100.0/81920<<"%)"<<std::endl; 
	}
}

void FenSaver::writeRes(Score res) {
	_stream <<'{'<<res<<'}';
	
}
