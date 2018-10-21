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

class SearchLimits
{
public:
	volatile bool ponder,infinite;
	unsigned int wtime,btime,winc,binc,movesToGo,nodes,mate,moveTime;
	int depth;
	std::list<Move> searchMoves;
	SearchLimits()
	{
		ponder = false;
		infinite = false;
		wtime = 0;
		btime = 0;
		winc = 0;
		binc = 0;
		movesToGo = 0;
		depth = -1;
		nodes = 0;
		mate = 0;
		moveTime = 0;
	}

};

#endif
