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
#ifndef ROOT_MOVE_H_
#define ROOT_MOVE_H_

#include "move.h"
#include "pvLine.h"
#include "score.h"


class rootMove
{
public:
	Score score = -SCORE_INFINITE;
	PVline PV;
	Move firstMove;
	unsigned int maxPlyReached = 0u;
	unsigned int depth = 0u;
	unsigned long long nodes = 0u;
	long long int time = 0ll;
	bool operator<(const rootMove& m) const { return score > m.score; } // Ascending sort
	bool operator==(const Move& m) const { return firstMove == m; }
	bool operator==(const rootMove& m) const { return firstMove == m.firstMove; }

	explicit rootMove(const Move& m) : firstMove{m}
	{
		PV.clear();
	}
	
	explicit rootMove( const Move& m, PVline& pv, Score s, unsigned int maxPly, unsigned int d, unsigned long long n, long long int t) : score{s}, PV{pv}, firstMove{m}, maxPlyReached{maxPly}, depth{d}, nodes{n}, time{t} {}
};

#endif