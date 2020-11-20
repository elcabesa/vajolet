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

#ifndef PERFT_H_
#define PERFT_H_

#include "transposition.h"

class Position;

class Perft
{
public:
	bool perftUseHash = false;
	
	explicit Perft( Position & pos, PerftTranspositionTable& tt): _pos(pos), _tt(tt){}
	unsigned long long perft(unsigned int depth);
	unsigned long long divide(unsigned int depth);

private:
	Position & _pos;
	PerftTranspositionTable& _tt;
	
};

#endif /* PERFT_H_ */