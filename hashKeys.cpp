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
#include "io.h"

U64  HashKeys::keys[squareNumber][30];  // position, piece (not all the keys are used)
U64  HashKeys::side;          // side to move (black)
U64  HashKeys::ep[squareNumber];        // ep targets (only 16 used)
U64  HashKeys::castlingRight[16];            // white king-side castling right
U64  HashKeys::exclusion;

/*!	\brief init the hashkeys
    \author Marco Belli
	\version 1.0
	\date 27/10/2013
 */
void HashKeys::init()
{
	// initialize all random 64-bit numbers
	int i,j;
	U64 temp[16];

	// use current time (in seconds) as random seed:
	srand(19091979);

	for (auto & val :ep){
		val = rand64();
	}

	for(auto & outerArray :keys)
	{
		for(auto & val :outerArray)
		{
			val= rand64();
		}

	}

	side = rand64();
	exclusion=rand64();

	for(auto & val :castlingRight ){
		val=0;
	}


	for(auto & val :temp){
		val=rand64();
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

/*!	\brief get a random 64 bit number
    \author Marco Belli
	\version 1.0
	\date 27/10/2013
 */
U64 HashKeys::rand64()
{
	return rand()^((U64)rand()<<15)^((U64)rand()<<30)^((U64)rand()<<45)^((U64)rand()<<60);
}


