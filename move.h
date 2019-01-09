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
#include <string>

#include "bitBoardIndex.h"
#include "score.h"
#include "tSquare.h"


/*!	\brief class move
	\author Marco Belli
	\version 1.0
	\date 22/02/2018
 */
class Move
{
public:
	enum eflags
	{
		fnone,
		fpromotion,
		fenpassant,
		fcastle,
	};

	enum epromotion
	{
		promQueen,
		promRook,
		promBishop,
		promKnight,
	};

	/*****************************************************************
	*	constructors
	******************************************************************/
	explicit Move(){}
	explicit Move( const unsigned short i ):_u(i){}
	Move( const tSquare _from, const tSquare _to, const eflags _flag=fnone, const epromotion _prom=promQueen ):_u(_from, _to, _flag, _prom){}

	/*****************************************************************
	*	Operators
	******************************************************************/
	inline bool operator == (const Move& d1) const { return _u._packed == d1._u._packed; }
	inline bool operator != (const Move& d1) const { return _u._packed != d1._u._packed; }
	inline Move& operator = (const unsigned short b) { _u._packed = b; return *this; }
	explicit operator bool() const { return _u._packed; }

	/*****************************************************************
	*	setter methods
	******************************************************************/
	void setFrom( tSquare from );
	void setTo( tSquare to );
	void setFlag( eflags fl );
	void clearFlag();
	void setPromotion( epromotion pr );

	/*****************************************************************
	*	getter methods
	******************************************************************/
	epromotion getPromotionType() const;
	tSquare getFrom() const;
	tSquare getTo() const;
	unsigned short getPacked() const;
	bitboardIndex getPromotedPiece() const;

	/*****************************************************************
	*	other methods
	******************************************************************/
	bool isPromotionMove() const;
	bool isCastleMove() const;
	bool isEnPassantMove() const;
	bool isStandardMove() const;
	bool isKingSideCastle() const;
	bool isQueenSideCastle() const;

	/*****************************************************************
	*	static methods
	******************************************************************/
	static const Move NOMOVE;

	/*****************************************************************
	*	private members
	******************************************************************/

protected:
	union t_u
	{
		t_u(){};
		t_u( unsigned short packed ):_packed(packed){}
		t_u( const tSquare _from, const tSquare _to, const eflags _flag=fnone, const epromotion _prom=promQueen ):_bit{(unsigned int)_from,(unsigned int)_to,_prom, _flag}{}
		struct
		{
			unsigned _from		:6;
			unsigned _to		:6;
			unsigned _promotion	:2;
			unsigned _flags		:2;
		}_bit;
		unsigned short _packed;
	}_u;


};

inline void Move::setFrom( tSquare from ){ _u._bit._from = from; }
inline void Move::setTo( tSquare to ){ _u._bit._to = to; }
inline void Move::setFlag( eflags fl ){ _u._bit._flags = fl; }
inline void Move::clearFlag(){ _u._bit._flags = fnone; }
inline void Move::setPromotion(epromotion pr){ _u._bit._promotion = pr; }

inline Move::epromotion Move::getPromotionType() const
{
	return (epromotion)( _u._bit._promotion );
}

inline bool Move::isPromotionMove() const
{
	return _u._bit._flags == Move::fpromotion;
}

inline bool Move::isCastleMove() const
{
	return _u._bit._flags == Move::fcastle;
}

inline bool Move::isEnPassantMove() const
{
	return _u._bit._flags == Move::fenpassant;
}

inline bool Move::isStandardMove() const
{
	return _u._bit._flags == Move::fnone;
}

inline tSquare Move::getFrom()const
{
	return tSquare( _u._bit._from );

}
inline tSquare Move::getTo()const
{
	return tSquare( _u._bit._to );
}

inline unsigned short Move::getPacked()const
{
	return _u._packed;
}

inline bool Move::isKingSideCastle() const
{
	return _u._bit._to > _u._bit._from;
}

inline bool Move::isQueenSideCastle() const
{
	return _u._bit._to < _u._bit._from;
}

inline bitboardIndex Move::getPromotedPiece() const
{
	return bitboardIndex( whiteQueens + getPromotionType() );
}



/*!	\brief struct extMove
	\author Marco Belli
	\version 1.0
	\date 22/02/2018
 */
class extMove: public Move
{

public:
	
	
	/*****************************************************************
	*	constructors
	******************************************************************/
	explicit extMove(){};
	explicit extMove( const Move& m ): Move(m){}
	explicit extMove( const unsigned short i ): Move(i){}
	extMove( const tSquare _from, const tSquare _to, const eflags _flag=fnone, const epromotion _prom=promQueen): Move( _from, _to, _flag, _prom ){}
	
	/*****************************************************************
	*	Operators
	******************************************************************/
	inline bool operator < ( const extMove& d1 ) const { return _score < d1._score;}
	inline extMove& operator = ( const Move& m ){ _u._packed = m.getPacked(); return *this; }
	
	
	/*****************************************************************
	*	setter methods
	******************************************************************/
	void setScore( const Score s );
	
private:
	Score _score;
};

void inline extMove::setScore( const Score  s){ _score = s;}


/*!	\brief return the offset of a pawn push
    \author Marco Belli
	\version 1.0
	\date 08/11/2013
 */
inline tSquare pawnPush(bool color){
	return color? sud:north;
}






#endif /* MOVE_H_ */
