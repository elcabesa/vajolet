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

#ifndef DATA_H_
#define DATA_H_

#include "bitops.h"
#include "eCastle.h"
#include "tSquare.h"



//------------------------------------------------
//enum
//------------------------------------------------

//------------------------------------------------
//	const
//------------------------------------------------

//------------------------------------------------
//	extern variables
//------------------------------------------------
// todo move them in position, eval or endgame
extern bitMap ISOLATED_PAWN[squareNumber];
extern bitMap PASSED_PAWN[2][squareNumber];
extern bitMap SQUARES_IN_FRONT_OF[2][squareNumber];
extern bitMap centerBitmap;
extern bitMap bigCenterBitmap;
extern bitMap spaceMask;



//------------------------------------------------
//	inline functions
//------------------------------------------------

/*	\brief set the Nth bit to 1
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bitMap bitSet(tSquare n)
{
	//extern bitMap BITSET[squareNumber+1];
	//return BITSET[n];
	return (1ull) << n;
}

inline bool isSquareSet( const bitMap b, const tSquare sq )
{
	return b & bitSet( sq );
}

/*	\brief return true if the 3 squares are aligned
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool squaresAligned(tSquare s1, tSquare s2, tSquare s3)
{
	extern bitMap LINES[squareNumber][squareNumber];
	return isSquareSet( LINES[s1][s2], s3 );
}

inline const bitMap& getSquaresBetween( const tSquare sq1, const tSquare sq2 )
{
	extern bitMap SQUARES_BETWEEN[squareNumber][squareNumber];
	return SQUARES_BETWEEN[sq1][sq2];
}

inline const bitMap& getColorBitmap( const Color c )
{
	extern bitMap BITMAP_COLOR[2];
	return BITMAP_COLOR[c];
}

inline unsigned int distance( const tSquare sq1, const tSquare sq2 )
{
	extern unsigned int SQUARE_DISTANCE[squareNumber][squareNumber];
	return SQUARE_DISTANCE[sq1][sq2];
}

inline const bitMap& rankMask( const tSquare sq )
{
	extern bitMap RANKMASK[squareNumber];
	return RANKMASK[sq];
}

inline const bitMap& fileMask( const tSquare sq )
{
	extern bitMap FILEMASK[squareNumber];
	return FILEMASK[sq];
}



//------------------------------------------------
//	function prototype
//------------------------------------------------
void initData(void);

#endif /* DATA_H_ */
