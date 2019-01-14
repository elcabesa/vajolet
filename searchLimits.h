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
	explicit SearchLimits() {}

	void checkInfiniteSearch()
	{
		if(_btime == -1 && _wtime == -1 && _moveTime == -1)
		{
			setInfiniteSearch();
		}
	}
	
	bool isMoveTimeSearch() const { return _moveTime != -1; }
	bool isNodeLimitedSearch() const { return _nodes != 0; } 
	bool isDepthLimitedSearch() const { return _depth != -1; }
	bool isSearchMovesMode() const { return _searchMoves.size() != 0; }
	bool isInfiniteSearch() const { return _infinite; }
	bool isPondering() const { return _ponder; }
	
	long long int getWTime() const { return _wtime; }
	void setWTime(long long int x) { _wtime = x; }
	long long int getBTime() const  { return _btime; }
	void setBTime(long long int x) { _btime = x; }
	long long int getWInc() const  { return _winc; }
	void setWInc(long long int x) { _winc = x; }
	long long int getBInc() const  { return _binc; }
	void setBInc(long long int x) { _binc = x; }
	long long int getMovesToGo() const  { return _movesToGo; }
	void setMovesToGo(long long int x) { _movesToGo = x; }
	int getDepth() const  { return _depth; }
	void setDepth(int x) { _depth = x; }
	long long int getMate() const  { return _mate; }
	void setMate(long long int x) { _mate = x; }
	
	long long int getMoveTime() const { return _moveTime; }
	void setMoveTime(long long int x) { _moveTime = x; }
	unsigned int getNodeLimit() const { return _nodes; }
	void setNodeLimit(long long int x) { _nodes = x; }
	
	void setInfiniteSearch(){ _infinite = true; }
	void setPonder(const bool p){ _ponder = p; }
	
	const std::list<Move>& getMoveList() const { return _searchMoves; }
	void moveListInsert( const Move& m){ _searchMoves.push_back(m); }
	
private:
	long long int _wtime = -1, _btime = -1, _winc = 0, _binc = 0, _movesToGo = 0, _mate = 0, _moveTime = -1;
	unsigned int _nodes = 0;
	int _depth = -1;
	std::list<Move> _searchMoves;
	bool _infinite = false;
	volatile bool _ponder = false;

};

#endif
