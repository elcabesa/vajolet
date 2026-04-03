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
#include "txtSaver.h"
#include "position.h"
#include "move.h"
#include "searchResult.h"
#include "timeManagement.h"
#include "transposition.h"


unsigned long long TxtSaver::_savedPositions = 0;
std::mutex TxtSaver::_mutex;

TxtSaver::TxtSaver(unsigned int decimation, std::ofstream& stream): _decimation(decimation), _stream(stream){

}

TxtSaver::~TxtSaver() {
	const std::lock_guard<std::mutex> lock(_mutex);
	_stream.write(_buffer.data(), _buffer.size());
	_stream.flush();
	_buffer.clear();
	_savedPositions += _saved;
	_saved = 0;
}


void TxtSaver::save(Position& pos, Score res) {
	if (++_counter >= _decimation) {
		_counter = 0;
		++_saved;
		if(pos.isBlackTurn()) {
			res *=-1;
		}
		auto s = pos.getFen();
		s +=" | ";
		s += std::to_string(res/100);
		s +=" | 0.5";
		s +="\n";

		for(auto& c: s) {
			_buffer.push_back(c);
		}

		if(_buffer.size()>1024) {
			const std::lock_guard<std::mutex> lock(_mutex);
			_stream.write(_buffer.data(), _buffer.size());
			_stream.flush();
			_buffer.clear();
			_savedPositions += _saved;
			_saved = 0;

		}
	}
}
