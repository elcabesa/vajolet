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

#include "data.h"

//--------------------------------------------------------------
//	global variables
//--------------------------------------------------------------
#ifdef PRECALCULATED_BITSET
bitMap BITSET[squareNumber];
#endif
unsigned int BOARDINDEX[8][8];

const int FILES[squareNumber] = {
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
};

const int RANKS[squareNumber] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
};

bitMap RANKMASK[squareNumber];
bitMap FILEMASK[squareNumber];


//--------------------------------------------------------------
//	function bodies
//--------------------------------------------------------------
void initData(void){
#ifdef PRECALCULATED_BITSET
	for(int i=0;i<squareNumber;i++){
		BITSET[i]=(1ull)<<i;
	}
#endif
	for(int i=0; i<squareNumber;i++){
		BOARDINDEX[i%8][i/8] = i;
	}

	for (int file = 0; file < 8; file++)
	{
		for (int rank = 0; rank < 8; rank++)
		{
			//===========================================================================
			//initialize 8-bit rank mask
			//===========================================================================

			RANKMASK[BOARDINDEX[file][rank]]  = bitSet(BOARDINDEX[0][rank]) | bitSet(BOARDINDEX[1][rank]) | bitSet(BOARDINDEX[2][rank]) | bitSet(BOARDINDEX[3][rank]) ;
			RANKMASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[4][rank]) | bitSet(BOARDINDEX[5][rank]) | bitSet(BOARDINDEX[6][rank]) | bitSet(BOARDINDEX[7][rank]) ;

			//===========================================================================
			//initialize 8-bit file mask
			//===========================================================================
			FILEMASK[BOARDINDEX[file][rank]]  = bitSet(BOARDINDEX[file][0]) | bitSet(BOARDINDEX[file][1]) | bitSet(BOARDINDEX[file][2]) | bitSet(BOARDINDEX[file][3]) ;
			FILEMASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[file][4]) | bitSet(BOARDINDEX[file][5]) | bitSet(BOARDINDEX[file][6]) | bitSet(BOARDINDEX[file][7]) ;
		}
	}
}

