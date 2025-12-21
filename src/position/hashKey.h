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
#ifndef HASHKEY_H_
#define HASHKEY_H_


//---------------------------------
//	includes
//---------------------------------
#include <cstdint>

#include "bitBoardIndex.h"
#include "tSquare.h"

using tKey = uint64_t;
//---------------------------------
//	structure
//---------------------------------
class HashKey
{
private:

	tKey _key;

	static tKey _keys[squareNumber][30];// position, piece (not all the keys are used)
	static tKey _side;					// side to move (black)
	static tKey _ep[squareNumber];		// ep targets (only 16 used)
	static tKey _castlingRight[16];		// white king-side castling right
	static tKey _exclusion;

public:
	static void init();       // initialize the random data
	
	size_t operator()(const HashKey& p) const {
		return p._key;
	}

	inline HashKey getExclusionKey() const
	{
		return HashKey( _key ^ _exclusion );
	}

	inline tKey getKey() const
	{
		return _key;
	}

	inline void updatePiece( const tSquare t, const bitboardIndex piece )
	{
		_key ^= _keys[ t ][ piece ];
	}

	inline void updatePiece( const tSquare from, const tSquare to, const bitboardIndex piece )
	{
		_key ^= _keys[ from ][ piece ] ^ _keys[ to ][ piece ];
	}

	inline void updatePiece( const tSquare t, const bitboardIndex piece1, const bitboardIndex piece2 )
	{
		_key ^= _keys[ t ][ piece1 ] ^ _keys[ t ][ piece2 ];
	}

	inline void changeSide()
	{
		_key ^= _side;
	}

	// todo use eCastle
	inline void setCastlingRight( const unsigned int c )
	{
		_key ^= _castlingRight[ c ];
	}

	inline void changeEp( const tSquare t )
	{
		_key ^= _ep[t];
	}

	explicit HashKey(){}
	explicit HashKey(tKey k):_key(k){}

	bool operator==(HashKey const & r) const {return (_key == r._key);}
	bool operator!=(HashKey const & r) const {return !(*this == r);}
};



#endif /* HASHKEY_H_ */
