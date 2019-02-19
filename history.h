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



#ifndef HISTORY_H_
#define HISTORY_H_

#include <cstdlib>
#include <cstring>
#include <array>

#include "bitBoardIndex.h"
#include "move.h"
#include "score.h"
#include "tSquare.h"
#include "vajolet.h"



class History
{
private:
	Score _table[2][squareNumber][squareNumber];
public :

	inline void clear() { std::memset(_table, 0, sizeof(_table)); }

	inline void update( const Color c, const Move& m, const Score v)
	{
		const int W = 32;
		const int D = 500;

		assert(c<=black);
		
		Score & e = _table[c][m.getFrom()][m.getTo()];
		e += v * W - e * std::abs(v)/ D;
	}

	inline Score getValue( const Color c, const Move& m ) const
	{
		assert(c<=black);
		return _table[c][m.getFrom()][m.getTo()];
	}

	explicit History(){}

};

class CaptureHistory
{
private:
	// piece, to, captured piece
	Score _table[lastBitboard][squareNumber][lastBitboard];
public :

	inline void clear() { std::memset(_table, 0, sizeof(_table)); }


	inline void update( const bitboardIndex p, const Move& m, const bitboardIndex captured, const Score v)
	{
				
		const int W = 2;
		const int D = 324;
		
		//if( captured == empty && m.bit.flags == Move::fenpassant )
		//{
		//	captured = ( isblack( p ) ? whitePawns : blackPawns );
		//}
		
		assert( isValidPiece( p ));
		assert( isValidPiece( captured ) || captured == empty );
		const tSquare to = (tSquare)m.getTo();

		Score &e = _table[p][to][captured];
		e += v * W - e * std::abs(v)/ D;
	}
	inline Score getValue( const bitboardIndex p, const Move& m, bitboardIndex captured ) const
	{
		//if( captured == empty && m.bit.flags == Move::fenpassant )
		//{
		//	captured = ( isblack( p ) ? whitePawns : blackPawns );
		//}
		assert( isValidPiece( p ) );
		assert( isValidPiece( captured ) || captured == empty );
		const tSquare to = (tSquare)m.getTo();
		return _table[p][to][captured];
	}


	explicit CaptureHistory(){}

};

class CounterMove
{
private:
	std::array<std::array<std::array<Move,2>,squareNumber>,lastBitboard> _table;
public :

	inline void clear()
	{
		for(auto& a: _table)
			for(auto& b: a)
				b.fill(Move::NOMOVE);
	}


	inline void update( const bitboardIndex p, const tSquare to, const Move m)
	{

		assert( isValidPiece( p ) );
		assert(to<squareNumber);
		auto& mm =  _table[p][to];
		if(mm[0] != m)
		{
			mm[1] = mm[0];
			mm[0] = m;
		}

	}
	inline const Move& getMove( const bitboardIndex p, const tSquare to, const unsigned int pos ) const
	{
		assert( pos < 2 );
		assert( isValidPiece( p ) );
		assert(to<squareNumber);
		return _table[p][to][pos];
	}


	explicit CounterMove(){}

};



#endif /* HISTORY_H_ */
