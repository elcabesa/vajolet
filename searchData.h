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
#ifndef SEARCH_DATA_H_
#define SEARCH_DATA_H_

#include "move.h"
#include "history.h"


class SearchData
{
public:
	struct Sd
	{
		Move excludeMove = Move(0);
		bool skipNullMove = false;
		Move killers[2] = { Move(0),Move(0)};
		Score staticEval = 0;
		bool inCheck = false;
	} story[800];
	CounterMove counterMoves;
	CaptureHistory captureHistory;
	History history;

	void clearKillers(unsigned int ply);
	void cleanData(void);
	void saveKillers(unsigned int ply, const Move& m);

	const History& getHistory()const {return history;}
	const CaptureHistory& getCaptureHistory()const {return captureHistory;}
	const CounterMove& getCounterMove()const {return  counterMoves;}
	const Move& getKillers(unsigned int ply,unsigned int n) const { return story[ply].killers[n]; }
};

extern const SearchData _defaultSearchData;

#endif