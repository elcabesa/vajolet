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
#ifndef PVLINE_H_
#define PVLINE_H_

#include <list>
#include "move.h"

class PVline : private std::list<Move>
{
public:
	inline unsigned int size() const
	{
		return std::list<Move>::size();
	}
	inline void clear()
	{
		std::list<Move>::clear();
	}
	
	inline void appendNewPvLine( const Move&  bestMove, PVline& childPV )
	{
		clear();
		emplace_back( bestMove );
		splice( end(), childPV );
	}
	
	inline void appendNewMove( const Move& move )
	{
		clear();
		emplace_back( move );
	}
	
	inline const Move& getMove( unsigned int n ) const
	{
		if( size() > n )
		{
			auto it = begin();
			std::advance(it, n);
			return *it;
		}
		else
		{
			return Move::NOMOVE;
		}
	}

	using std::list<Move>::iterator;
	using std::list<Move>::begin;
	using std::list<Move>::end;
	explicit PVline( unsigned int n, const Move m ) : std::list<Move>(n,m){}
	explicit PVline(){}
	
};

#endif