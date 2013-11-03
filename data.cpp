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

bitMap DIAGA1H8MASK[squareNumber];
bitMap DIAGA8H1MASK[squareNumber];
bitMap SQUARES_BETWEEN[squareNumber][squareNumber];

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
		DIAGA1H8MASK[i]=0;
		DIAGA8H1MASK[i]=0;
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

			//===========================================================================
			//Initialize 8-bit diagonal mask
			//===========================================================================
			int diaga8h1 = file + rank; // from 0 to 14, longest diagonal = 7
			if (diaga8h1 < 8)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[square][diaga8h1-square]);
				}
			}
			else  // upper half, diagonals 8 to 14
			{
				for (int square = 0 ; square < 15 - diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[diaga8h1+square-7][7-square]);
				}
			}


			//===========================================================================
			//Initialize 8-bit diagonal mask, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			int diaga1h8 = file - rank; // from -7 to +7, longest diagonal = 0
			if (diaga1h8 > -1)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= 7 - diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[diaga1h8 + square][square]);
				}
			}
			else
			{
				for (int square = 0 ; square <= 7 + diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[square][square - diaga1h8]);
				}
			}

		}
	}

	for(int square=0;square<squareNumber;square++){
		for(int i=0;i<squareNumber;i++){
			SQUARES_BETWEEN[square][i]=0;
		}
	}

	for(int square=0;square<squareNumber;square++){
		for(int i=0;i<64;i++){
			if(square==i){
				SQUARES_BETWEEN[square][i]=0;
			}
			if(FILES[square]==FILES[i]){
				// stessa colonna
				if(RANKS[i]>RANKS[square]){ // in salita
					int temp=RANKS[square]+1;
					while(temp<RANKS[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[FILES[square]][temp]);
						temp++;
					}
				}
				if(RANKS[i]<RANKS[square]){ // in discesa
					int temp=RANKS[square]-1;
					while(temp>RANKS[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[FILES[square]][temp]);
						temp--;
					}
				}
			}
			if(RANKS[square]==RANKS[i]){/// stessa riga
				if(FILES[i]>FILES[square]){ // in salita
					int temp=FILES[square]+1;
					while(temp<FILES[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[temp][RANKS[square]]);
						temp++;
					}
				}
				if(FILES[i]<FILES[square]){ // in discesa
					int temp=FILES[square]-1;
					while(temp>FILES[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[temp][RANKS[square]]);
						temp--;
					}
				}
			}
			if(DIAGA1H8MASK[square]& BITSET[i]){
				if(FILES[i]>FILES[square]){ // in salita
					int temp=FILES[square]+1;
					int temp2=RANKS[square]+1;
					while(temp<FILES[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[temp][temp2]);
						temp++;
						temp2++;
					}
				}
				if(FILES[i]<FILES[square]){ // in discesa
					int temp=FILES[square]-1;
					int temp2=RANKS[square]-1;
					while(temp>FILES[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[temp][temp2]);
						temp--;
						temp2--;
					}
				}
			}
			if(DIAGA8H1MASK[square]& BITSET[i]){
				if(FILES[i]>FILES[square]){ // in salita
					int temp=FILES[square]+1;
					int temp2=RANKS[square]-1;
					while(temp<FILES[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[temp][temp2]);
						temp++;
						temp2--;
					}
				}
				if(FILES[i]<FILES[square]){ // in discesa
					int temp=FILES[square]-1;
					int temp2=RANKS[square]+1;
					while(temp>FILES[i]){
						SQUARES_BETWEEN[square][i]|=bitSet(BOARDINDEX[temp][temp2]);
						temp--;
						temp2++;
					}
				}
			}
		}
	}


}

