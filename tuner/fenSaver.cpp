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

FenSaver::FenSaver(unsigned int decimation): _decimation(decimation), _player("p"){
	_stream.open("fen.csv");
	_sl.setDepth(4);
}


void FenSaver::save(const Position& pos) {
	if (++_counter >= _decimation) {
        auto res = _player.doSearch(pos, _sl).Res;
        auto eval = pos.eval<false>();
        if (std::abs(res)< SCORE_KNOWN_WIN && std::abs(eval)< SCORE_KNOWN_WIN) {
		    _counter = 0;
		    std::cout << "saved " << ++_saved << " FENs" <<std::endl;
		    _stream << pos.getFen() << "," << eval << "," << res << std::endl;
        }
		
	}
}