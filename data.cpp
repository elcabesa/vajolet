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
#include "bitops.h"
#include "tSquare.h"
#include <algorithm>

//--------------------------------------------------------------
//	global variables
//--------------------------------------------------------------

const volatile tFile FILES[squareNumber] = {		//!< precalculated file from square number
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH,
	FILEA, FILEB, FILEC, FILED, FILEE, FILEF, FILEG, FILEH
};

const volatile tRank RANKS[squareNumber] = {		//!< precalculated rank from square number
	RANK1, RANK1, RANK1, RANK1, RANK1, RANK1, RANK1, RANK1,
	RANK2, RANK2, RANK2, RANK2, RANK2, RANK2, RANK2, RANK2,
	RANK3, RANK3, RANK3, RANK3, RANK3, RANK3, RANK3, RANK3,
	RANK4, RANK4, RANK4, RANK4, RANK4, RANK4, RANK4, RANK4,
	RANK5, RANK5, RANK5, RANK5, RANK5, RANK5, RANK5, RANK5,
	RANK6, RANK6, RANK6, RANK6, RANK6, RANK6, RANK6, RANK6,
	RANK7, RANK7, RANK7, RANK7, RANK7, RANK7, RANK7, RANK7,
	RANK8, RANK8, RANK8, RANK8, RANK8, RANK8, RANK8, RANK8,

};

int SQUARE_COLOR[squareNumber]=
{
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0,
	0,1,0,1,0,1,0,1,
	1,0,1,0,1,0,1,0
};

bitMap BITMAP_COLOR[2];

bitMap RANKMASK[squareNumber];			//!< bitmask of a rank given a square on the rank
bitMap FILEMASK[squareNumber];			//!< bitmask of a file given a square on the rank

bitMap DIAGA1H8MASK[squareNumber];		//!< bitmask of a diagonal given a square on the diagonal
bitMap DIAGA8H1MASK[squareNumber];		//!< bitmask of a diagonal given a square on the diagonal
bitMap SQUARES_BETWEEN[squareNumber][squareNumber];		//bitmask with the squares btween 2 alinged squares, 0 otherwise
bitMap LINES[squareNumber][squareNumber];

bitMap ISOLATED_PAWN[squareNumber];
bitMap PASSED_PAWN[2][squareNumber];
bitMap SQUARES_IN_FRONT_OF[2][squareNumber];

unsigned int SQUARE_DISTANCE[squareNumber][squareNumber];

bitMap centerBitmap;
bitMap bigCenterBitmap;
bitMap spaceMask;

//--------------------------------------------------------------
//	function bodies
//--------------------------------------------------------------

/*	\brief init global data structures
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
void initData(void)
{
	for(tSquare i = A1; i<squareNumber; i++)
	{
		DIAGA1H8MASK[i] = 0;
		DIAGA8H1MASK[i] = 0;
	}

	centerBitmap = bitSet(E4)|bitSet(E5)|bitSet(D4)|bitSet(D5);
	bigCenterBitmap =
			bitSet(C6)|bitSet(D6)|bitSet(E6)|bitSet(F6)|
			bitSet(C5)|bitSet(C4)|bitSet(F5)|bitSet(F4)|
			bitSet(C3)|bitSet(D3)|bitSet(E3)|bitSet(F3);


	for (tFile file = FILEA; file <= FILEH; file++)
	{
		for (tRank rank = RANK1; rank <= RANK8; rank++)
		{
			//===========================================================================
			//initialize 8-bit rank mask
			//===========================================================================

			RANKMASK[getSquare(file,rank)]  = bitSet(getSquare(FILEA,rank)) | bitSet(getSquare(FILEB,rank)) | bitSet(getSquare(FILEC,rank)) | bitSet(getSquare(FILED,rank)) ;
			RANKMASK[getSquare(file,rank)] |= bitSet(getSquare(FILEE,rank)) | bitSet(getSquare(FILEF,rank)) | bitSet(getSquare(FILEG,rank)) | bitSet(getSquare(FILEH,rank)) ;

			//===========================================================================
			//initialize 8-bit file mask
			//===========================================================================
			FILEMASK[getSquare(file,rank)]  = bitSet(getSquare(file,RANK1)) | bitSet(getSquare(file,RANK2)) | bitSet(getSquare(file,RANK3)) | bitSet(getSquare(file,RANK4)) ;
			FILEMASK[getSquare(file,rank)] |= bitSet(getSquare(file,RANK5)) | bitSet(getSquare(file,RANK6)) | bitSet(getSquare(file,RANK7)) | bitSet(getSquare(file,RANK8)) ;

			//===========================================================================
			//Initialize 8-bit diagonal mask
			//===========================================================================
			int diaga8h1 = file + rank; // from 0 to 14, longest diagonal = 7
			if (diaga8h1 < 8)  // lower half, diagonals 0 to 7
			{
				for (int square = 0 ; square <= diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[getSquare(file,rank)] |= bitSet(getSquare(tFile(square),tRank(diaga8h1-square)));
				}
			}
			else  // upper half, diagonals 8 to 14
			{
				for (int square = 0 ; square < 15 - diaga8h1 ; square ++)
				{
					DIAGA8H1MASK[getSquare(file,rank)] |= bitSet(getSquare(tFile(diaga8h1+square-7),tRank(7-square)));
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
					DIAGA1H8MASK[getSquare(file,rank)] |= bitSet(getSquare(tFile(diaga1h8 + square),tRank(square)));
				}
			}
			else
			{
				for (int square = 0 ; square <= 7 + diaga1h8 ; square ++)
				{
					DIAGA1H8MASK[getSquare(file,rank)] |= bitSet(getSquare(tFile(square),tRank(square - diaga1h8)));
				}
			}

		}
	}
	
	for(tSquare square=A1; square<squareNumber; square++)
	{
		for(tSquare i=A1; i<squareNumber; i++)
		{
			LINES[square][i] =0;
			SQUARES_BETWEEN[square][i] = 0;

			if(i == square)
			{
				continue;
			}

			if(getFileOf(square) == getFileOf(i))
			{
				// stessa colonna

				LINES[square][i] = fileMask(square);
				if(getRankOf(i) > getRankOf(square)) // in salita
				{
					tRank temp = tRank(getRankOf(square) + 1);
					while(temp < getRankOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(getFileOf(square),temp));
						temp++;
					}
				}
				if(getRankOf(i) < getRankOf(square)) // in discesa
				{
					tRank temp = tRank(getRankOf(square) - 1);
					while(temp > getRankOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(getFileOf(square),temp));
						temp--;
					}
				}
			}
			if(getRankOf(square) == getRankOf(i)) // stessa riga
			{
				LINES[square][i] = rankMask(square);
				if(getFileOf(i) > getFileOf(square)) // in salita
				{
					tFile temp = tFile(getFileOf(square) + 1);
					while(temp < getFileOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(temp,getRankOf(square)));
						temp++;
					}
				}
				if(getFileOf(i) < getFileOf(square)) // in discesa
				{
					tFile temp = tFile(getFileOf(square) - 1);
					while(temp > getFileOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(temp,getRankOf(square)));
						temp--;
					}
				}
			}
			if( isSquareSet( DIAGA1H8MASK[square], i ) )
			{
				LINES[square][i] = DIAGA1H8MASK[square];
				if(getFileOf(i) > getFileOf(square)) // in salita
				{
					tFile temp = tFile(getFileOf(square) + 1);
					tRank temp2 = tRank(getRankOf(square) + 1);
					while(temp < getFileOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(temp,temp2));
						temp++;
						temp2++;
					}
				}
				if(getFileOf(i) < getFileOf(square)) // in discesa
				{
					tFile temp = tFile(getFileOf(square) - 1);
					tRank temp2 = tRank(getRankOf(square)- 1);
					while(temp > getFileOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(temp,temp2));
						temp--;
						temp2--;
					}
				}
			}
			if( isSquareSet( DIAGA8H1MASK[square], i ) )
			{
				LINES[square][i] = DIAGA8H1MASK[square];
				if(getFileOf(i) > getFileOf(square)) // in salita
				{
					tFile temp = tFile(getFileOf(square) + 1);
					tRank temp2 = tRank(getRankOf(square) - 1);
					while(temp < getFileOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(temp,temp2));
						temp++;
						temp2--;
					}
				}
				if(getFileOf(i) < getFileOf(square)) // in discesa
				{
					tFile temp = tFile(getFileOf(square)-1);
					tRank temp2 = tRank(getRankOf(square)+1);
					while(temp > getFileOf(i))
					{
						SQUARES_BETWEEN[square][i] |= bitSet(getSquare(temp,temp2));
						temp--;
						temp2++;
					}
				}
			}
		}
	}

	//////////////////////////////////////////////////
	//
	//	PAWN STRUCTURE DATA
	//
	//////////////////////////////////////////////////
	for(tSquare square = A1; square < squareNumber; square++)
	{
		ISOLATED_PAWN[square] = 0;
		tFile file = getFileOf(square);

		if(file>FILEA)
		{
			ISOLATED_PAWN[square] |= fileMask(getSquare(tFile(file-1),RANK1));
		}
		if(file<FILEH)
		{
			ISOLATED_PAWN[square] |= fileMask(getSquare(tFile(file+1),RANK1));
		}
	}

	for(tSquare square = A1; square < squareNumber; square++)
	{
		PASSED_PAWN[0][square] = 0;
		PASSED_PAWN[1][square] = 0;
		SQUARES_IN_FRONT_OF[0][square] = 0;
		SQUARES_IN_FRONT_OF[1][square] = 0;
		tFile file = getFileOf(square);
		tRank rank = getRankOf(square);

		for(tRank i=tRank(rank+1); i<=RANK8; i++)
		{
			if(file>FILEA)
			{
				PASSED_PAWN[0][square] |= bitSet(getSquare(tFile(file-1),i));
			}
			PASSED_PAWN[0][square] |= bitSet(getSquare(file,i));
			SQUARES_IN_FRONT_OF[0][square] |= bitSet(getSquare(file,i));
			if(file<FILEH)
			{
				PASSED_PAWN[0][square] |= bitSet(getSquare(tFile(file+1),i));
			}
		}

		for(tRank i=tRank(rank-1); i>=RANK1; i--)
		{
			if(file>FILEA)
			{
				PASSED_PAWN[1][square] |= bitSet(getSquare(tFile(file-1),i));
			}
			PASSED_PAWN[1][square] |= bitSet(getSquare(file,i));
			SQUARES_IN_FRONT_OF[1][square] |= bitSet(getSquare(file,i));
			if(file<FILEH)
			{
				PASSED_PAWN[1][square] |= bitSet(getSquare(tFile(file+1),i));
			}
		}
	}

	for(tSquare square = A1; square < squareNumber; square++)
	{
		for(tSquare square2 = A1; square2 < squareNumber; square2++){

			SQUARE_DISTANCE[square][square2] = std::max(std::abs(getFileOf(square)-getFileOf(square2)), std::abs(getRankOf(square)-getRankOf(square2)));
		}
	}

	BITMAP_COLOR[0] = 0;
	BITMAP_COLOR[1] = 0;

	for(tSquare square = A1; square < squareNumber; square++)
	{
		if( getSquareColor(square))
		{
			BITMAP_COLOR[1] |= bitSet(square);
		}
		else
		{
			BITMAP_COLOR[0] |= bitSet(square);
		}
	}
	
	spaceMask = fileMask(C1) | fileMask(D1) | fileMask(E1) | fileMask(F1);
}

