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
	long long int _wtime, _btime, _winc, _binc, _movesToGo, _mate, _moveTime;
	unsigned int _nodes;
	int _depth;
	std::list<Move> searchMoves;
	explicit SearchLimits()
	{
		ponder = false;
		infinite = false;
		_wtime = -1;
		_btime = -1;
		_winc = 0;
		_binc = 0;
		_movesToGo = 0;
		_depth = -1;
		_mate = 0;
		_moveTime = -1;
		_nodes = 0;
	}

	void checkInfiniteSearch()
	{
		if(_btime == -1 && _wtime == -1 && _moveTime == -1)
		{
			infinite = true;
		}
	}
	
	bool isMoveTimeSearch() const { return _moveTime != -1; }
	bool isNodeLimitedSearch() const { return _nodes != 0; } 
	bool isDepthLimitedSearch() const { return _depth != -1; }
	
	long long int getWTime() const { return _wtime; }
	long long int& getWTime() { return _wtime; }
	long long int getBTime() const  { return _btime; }
	long long int& getBTime() { return _btime; }
	long long int getWInc() const  { return _winc; }
	long long int& getWInc() { return _winc; }
	long long int getBInc() const  { return _binc; }
	long long int& getBInc() { return _binc; }
	long long int getMovesToGo() const  { return _movesToGo; }
	long long int& getMovesToGo() { return _movesToGo; }
	int getDepth() const  { return _depth; }
	int& getDepth() { return _depth; }
	long long int getMate() const  { return _mate; }
	long long int& getMate() { return _mate; }
	
	long long int getMoveTime() const { return _moveTime; }
	long long int& getMoveTime() { return _moveTime; }
	unsigned int getNodeLimit() const { return _nodes; }
	unsigned int& getNodeLimit() { return _nodes; }

};

#endif
