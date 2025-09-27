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

#include "epdSaver.h"
#include "position.h"
#include "move.h"
#include "searchResult.h"
#include "timeManagement.h"
#include "transposition.h"

#define DEBUG_PROCESS


EpdSaver::EpdSaver(unsigned int decimation, unsigned int n): _decimation(decimation){
	_stream.open("fen"+ std::to_string(n) + ".epd", std::ios_base::app);
#ifdef DEBUG_PROCESS
	_stream2.open("fen"+ std::to_string(n) + "ver.epd", std::ios_base::app);
#endif
}


void EpdSaver::save(const Position& pos, Score res) {
	save(pos, Move::NOMOVE, res);
}

void EpdSaver::save(const Position& pos, const Move& m, Score res) {
	if (++_counter >= _decimation) {
		_counter = 0;
		++_logDecimationCnt;
		++_saved;
		if(_logDecimationCnt>=10000) {
			std::cout << "saved " << _saved << " FENs" <<std::endl;
			_logDecimationCnt = 0;
		}
		if(m != Move::NOMOVE) {
			_stream << m.getPacked()<<";"<<res<< std::endl;
		} else {
			_stream << pos.getFen()<<";"<<res<< std::endl;
		}
#ifdef DEBUG_PROCESS
		_stream2 << pos.getFen()<<";"<<res<< std::endl;
#endif
	}
}
