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
#ifndef BINSAVER_H_
#define BINSAVER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>


#include "searchLimits.h"
#include "searchTimer.h"
#include "search.h"
#include "transposition.h"

class Position;
class Move;

class BinSaver {
	
public:
	BinSaver(unsigned int decimation);
    ~BinSaver();
	void save(Position& pos, Score res);
    unsigned long long getSavedPosition() const {
        const std::lock_guard<std::mutex> lock(_mutex);
        return _savedPositions;

    }

private:
    mutable std::mutex _mutex;
	const unsigned int _decimation;
	unsigned int _counter = 0;
	std::ofstream _stream;
	uint_fast64_t _saved = 0;
	unsigned int _logDecimationCnt = 0;
    std::vector<char> _buffer;
    unsigned long long _savedPositions = 0;
};

#endif /* BINSAVER_H_ */
