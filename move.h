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

#ifndef MOVE_H_
#define MOVE_H_

#include "tSquare.h"
#include "score.h"

/*!	\brief struct move
    \author Marco Belli
	\version 1.0
	\date 08/11/2013
 */
struct Move
{
	
	Move(){}
	Move(const Move& m): packed(m.packed){}
	Move(unsigned short i):packed(i){}
	Move(unsigned short _from, unsigned short _to, unsigned short _flag=fnone, unsigned short _prom=promQueen):bit{_from,_to,_prom, _flag}{}
	union
	{
		struct
		{
			unsigned from		:6;
			unsigned to			:6;
			unsigned promotion	:2;
			unsigned flags		:2;
		}bit;
		unsigned short packed;
	};
	enum eflags{
		fnone,
		fpromotion,
		fenpassant,
		fcastle,
	};

	enum epromotion{
		promQueen,
		promRook,
		promBishop,
		promKnight,
	};


	inline bool operator == (const Move& d1) const { return packed == d1.packed;}
	inline bool operator != (const Move& d1) const { return packed != d1.packed;}
	inline Move& operator = (unsigned short b) { packed = b; return *this;}
	inline Move& operator = (const Move&m){ packed = m.packed; return *this;}

	inline bool isPromotionMove() const
	{
		return bit.flags == Move::fpromotion;
	}
	inline bool isCastleMove() const
	{
		return bit.flags == Move::fcastle;
	}
	inline bool isEnPassantMove() const
	{
		return bit.flags == Move::fenpassant;
	}



};

struct extMove{
	Move m;
	Score score;

	extMove(){};
	inline bool operator < (const extMove& d1) const { return score < d1.score;}
	inline bool operator == (const Move& d1) const { return m == d1;}
	inline extMove& operator = ( const Move& mm ){ m.packed = mm.packed; return *this; }
};

/*!	\brief return the offset of a pawn push
    \author Marco Belli
	\version 1.0
	\date 08/11/2013
 */
inline tSquare pawnPush(int color){
	return color? sud:north;
}

const static Move NOMOVE = Move(0);





#endif /* MOVE_H_ */
