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
extern tSquare BOARDINDEX[8][8];
extern const int FILES[squareNumber];
extern const int RANKS[squareNumber];
extern bitMap RANKMASK[squareNumber];
extern bitMap FILEMASK[squareNumber];
extern bitMap SQUARES_BETWEEN[squareNumber][squareNumber];
extern bitMap ISOLATED_PAWN[squareNumber];
extern bitMap PASSED_PAWN[2][squareNumber];
extern bitMap SQUARES_IN_FRONT_OF[2][squareNumber];
extern const int SQUARE_COLOR[squareNumber];
extern bitMap BITMAP_COLOR[2];
extern int SQUARE_DISTANCE[squareNumber][squareNumber];
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
	extern bitMap BITSET[squareNumber+1];
	return BITSET[n];
}

/*	\brief return true if the 3 squares are aligned
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool squaresAligned(tSquare s1, tSquare s2, tSquare s3)
{
	extern bitMap LINES[squareNumber][squareNumber];
	return LINES[s1][s2] & bitSet(s3);
}
//------------------------------------------------
//	function prototype
//------------------------------------------------
void initData(void);

#endif /* DATA_H_ */
