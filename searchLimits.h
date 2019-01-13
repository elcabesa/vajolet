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

#ifndef SEARCH_LIMITS_H_
#define SEARCH_LIMITS_H_

#include <list>

#include "move.h"


class SearchLimits
{
public:
	volatile bool ponder, infinite;
	long long int wtime, btime, winc, binc, movesToGo, mate, moveTime;
	unsigned int nodes;
	int depth;
	std::list<Move> searchMoves;
	explicit SearchLimits()
	{
		ponder = false;
		infinite = false;
		wtime = -1;
		btime = -1;
		winc = 0;
		binc = 0;
		movesToGo = 0;
		depth = -1;
		mate = 0;
		moveTime = -1;
		nodes = 0;
	}

	void checkInfiniteSearch()
	{
		if(btime == -1 && wtime == -1 && moveTime == -1)
		{
			infinite = true;
		}
	}

};

#endif
