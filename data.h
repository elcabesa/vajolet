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
#include "vajolet.h"
#include "vectorclass/vectorclass.h"

//------------------------------------------------
//enum
//------------------------------------------------



//------------------------------------------------
//	const
//------------------------------------------------

//------------------------------------------------
//	extern variables
//------------------------------------------------
#ifdef PRECALCULATED_BITSET
extern bitMap BITSET[squareNumber+1];
#endif
extern tSquare BOARDINDEX[8][8];
extern const int FILES[squareNumber];
extern const int RANKS[squareNumber];
extern bitMap RANKMASK[squareNumber];
extern bitMap FILEMASK[squareNumber];
extern bitMap DIAGA1H8MASK[squareNumber];
extern bitMap DIAGA8H1MASK[squareNumber];
extern bitMap SQUARES_BETWEEN[squareNumber][squareNumber];
extern bitMap LINES[squareNumber][squareNumber];
extern bitMap ISOLATED_PAWN[squareNumber];
extern bitMap PASSED_PAWN[2][squareNumber];
extern bitMap SQUARES_IN_FRONT_OF[2][squareNumber];
extern const int SQUARE_COLOR[squareNumber];
extern bitMap BITMAP_COLOR[2];
extern int SQUARE_DISTANCE[squareNumber][squareNumber];
extern bitMap centerBitmap;
extern bitMap bigCenterBitmap;



//------------------------------------------------
//	inline functions
//------------------------------------------------

/*	\brief set the Nth bit to 1
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bitMap bitSet(tSquare n){
#ifdef PRECALCULATED_BITSET
	return BITSET[n];
#else
	//return (Vec2uq(1,0)<<n)[0];
	return (1ull)<<n;
#endif
}

/*	\brief return true if the 3 squares are aligned
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool squaresAligned(tSquare s1, tSquare s2, tSquare s3){
	return LINES[s1][s2] & bitSet(s3);
	/*return  (SQUARES_BETWEEN[s1][s2] | SQUARES_BETWEEN[s1][s3] | SQUARES_BETWEEN[s2][s3])
			& (     bitSet(s1) |        bitSet(s2) |        bitSet(s3));*/
}
//------------------------------------------------
//	function prototype
//------------------------------------------------
void initData(void);

#endif /* DATA_H_ */
