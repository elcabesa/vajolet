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

void BitWriter::add(int32_t number, unsigned int bits) {
	for(unsigned int i = 0; i < bits; ++i) {
		_buffer.push_back((number>>i)&1);
	}
}
void BitWriter::pad() {
	unsigned int b = _buffer.size() %8;
	if( b!=0) {
		unsigned int pad = 8 - b;
		for(unsigned int i = 0; i < pad; ++i) {
			_buffer.push_back(0);
		}
	}
}

std::vector<char> BitWriter::get() const {
	unsigned int bitPosition = 0;
	std::vector<char> out;
	for(auto b : _buffer) {
		if(bitPosition == 0) {
			out.push_back(0);
		}
		auto& c = out.back();
		c |= (b << bitPosition);
		bitPosition = (bitPosition + 1) % 8;
	}
	return out;
}

unsigned long long BinSaver::_savedPositions = 0;
std::mutex BinSaver::_mutex;

BinSaver::BinSaver(unsigned int decimation, std::ofstream& stream): _decimation(decimation), _stream(stream){

}

BinSaver::~BinSaver() {
	const std::lock_guard<std::mutex> lock(_mutex);
	_stream.write(_buffer.data(), _buffer.size());
	_stream.flush();
	_buffer.clear();
	_savedPositions += _saved;
	_saved = 0;
}


void BinSaver::save(Position& pos, Score res) {
	if (++_counter >= _decimation) {
		_counter = 0;
		//++_logDecimationCnt;
		++_saved;
		/*if(_logDecimationCnt>=10000) {
			std::cout << "saved " << _saved << " FENs" <<std::endl;
			_logDecimationCnt = 0;
		}*/

		pos.nnue()->clean();
		auto f = pos.nnue()->features();
		//_buffer.push_back(f.size());

		_bw.add(f.size(), 8);

		for(auto& idx: f) {
			//std::cout <<idx<<" ";
			/*union _bb{
				int16_t d;
				char c[2];
			}bb;
			bb.d = idx;
			_buffer.push_back(bb.c[0]);
			_buffer.push_back(bb.c[1]);*/

			_bw.add(idx, 10);

		}
		_bw.pad();
		/*float dval = res/50000.0;
		dval = 1.0/(1 + std::exp(-1.0 * dval)) ;*/
		float dval = res;
		//std::cout<<";"<<dval<<std::endl;
		union _bb{
			float d;
			//char c[4];
			uint32_t packed;
		}bb;
		bb.d = dval;
		/*_buffer.push_back(bb.c[0]);
		_buffer.push_back(bb.c[1]);
		_buffer.push_back(bb.c[2]);
		_buffer.push_back(bb.c[3]);*/
		_bw.add(bb.packed, 32);

		if(_bw.size()>1024) {
			const std::lock_guard<std::mutex> lock(_mutex);
			auto buff = _bw.get();
			_stream.write(buff.data(), buff.size());
			_stream.flush();
			_bw.clear();
			_savedPositions += _saved;
			_saved = 0;

		}
	}
}
