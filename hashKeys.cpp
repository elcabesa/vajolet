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

#include "hashKeys.h"

U64  HashKeys::keys[squareNumber][30];  // position, piece (not all the keys are used)
U64  HashKeys::side;          // side to move (black)
U64  HashKeys::ep[squareNumber];        // ep targets (only 16 used)
U64  HashKeys::castlingRight[16];            // white king-side castling right
U64  HashKeys::exclusion;


void HashKeys::init()
{
	// initialize all random 64-bit numbers

	int i,j;


	// use current time (in seconds) as random seed:
	srand(19091979);

	for (i = 0; i < 64; i++)
	{
		ep[i] = rand64();
		for (j=0; j < 30; j++) keys[i][j] = rand64();
	}
	side = rand64();
	exclusion=rand64();
	U64 temp[16];
	for(i=0;i<16;i++){
		temp[i]=rand64();
		castlingRight[i]=0;
	}
	for(i=0;i<16;i++){
		for(j=0;j<4;j++){
			if(i&(1<<j)){
				castlingRight[i]^=temp[i];
			}
		}
	}


	return;
}

U64 HashKeys::rand64()
{
	return rand()^((U64)rand()<<15)^((U64)rand()<<30)^((U64)rand()<<45)^((U64)rand()<<60);
}


