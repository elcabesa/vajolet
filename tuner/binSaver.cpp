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
#include "nnue.h"
#include "binSaver.h"
#include "position.h"
#include "move.h"
#include "searchResult.h"
#include "timeManagement.h"
#include "transposition.h"

#define DEBUG_PROCESS


BinSaver::BinSaver(unsigned int decimation, unsigned int n): _decimation(decimation){
	_stream.open("fen"+ std::to_string(n) + ".epd", std::ios_base::app);
#ifdef DEBUG_PROCESS
	_stream2.open("fen"+ std::to_string(n) + "ver.epd", std::ios_base::app);
#endif
}


void BinSaver::save(Position& pos, Score res) {
	char buffer[10];
	if (++_counter >= _decimation) {
		_counter = 0;
		++_logDecimationCnt;
		++_saved;
		if(_logDecimationCnt>=10000) {
			std::cout << "saved " << _saved << " FENs" <<std::endl;
			_logDecimationCnt = 0;
		}

		pos.nnue()->clean();
		auto f = pos.nnue()->features();
		buffer[0] = f.size();
		_stream.write(buffer,1);
		for(auto& idx: f) {
			//std::cout <<idx<<" ";
			union _bb{
				int16_t d;
				char c[2];
			}bb;
			bb.d = idx;
			_stream.write(bb.c,2);

		}
		/*float dval = res/50000.0;
		dval = 1.0/(1 + std::exp(-1.0 * dval)) ;*/
		float dval = res;
		//std::cout<<";"<<dval<<std::endl;
		union _bb{
			float d;
			char c[4];
		}bb;
		bb.d = dval;
		_stream.write(bb.c,4);
		_stream.flush();


#ifdef DEBUG_PROCESS
		_stream2 << pos.getFen()<<";"<<res<< std::endl;
#endif
	}
}
