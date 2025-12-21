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
#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>
#include "parameters.h"
#include "searchResult.h"
//#include "thread.h"
#include "searchLimits.h"
#include "searchTimer.h"
#include "search.h"
#include "transposition.h"

class Position;

class Player {
public:

	Player(std::string name);
	const SearchParameters& getSearchParametersConst() const;
	SearchParameters& getSearchParameters();
	const EvalParameters& getEvalParametersConst() const;
	EvalParameters& getEvalParameters();
	void insertResult(int res);
	std::string print() const;
	const std::string& getName() const;
	double pointRatio() const;
	SearchResult doSearch(const Position& p, SearchLimits& sl);
	
private:
	SearchParameters _sp;
	EvalParameters _ep;
	int _win = 0;
	int _lost = 0;
	int _draw = 0;
	int _unknown = 0;
	std::string _name;
	SearchLimits _sl;
	SearchTimer _st;
	transpositionTable _tt;
	Search _src;
	//my_thread _thr;
};

#endif /* PLAYER_H_ */
