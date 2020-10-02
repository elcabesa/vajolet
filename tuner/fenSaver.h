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
#ifndef FENSAVER_H_
#define FENSAVER_H_

#include <iostream>
#include <fstream>

#include "player.h"
#include "searchLimits.h"

class Position;

class FenSaver {
	
public:
	FenSaver(unsigned int decimation);
	void save(const Position& pos);

private:	
	const unsigned int _decimation;
	unsigned int _counter = 0;
	std::ofstream _stream;
	Player _player;
	SearchLimits _sl;
	uint_fast64_t _saved = 0;
};

#endif /* SELFPLAY_H_ */