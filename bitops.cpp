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
#include "bitops.h"
#include "data.h"
#include "io.h"


/*! \brief display a bitmap on stdout
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
void displayBitmap(bitMap b)
{
	char boardc[squareNumber];

	for ( tSquare i = A1; i < squareNumber; i++)
	{
		if (b & bitSet(i))
		{
			boardc[i] = '1';
		}
		else
		{
			boardc[i] = '.';
		}
	}

	sync_cout;
	for (tRank rank = RANK8 ; rank >= RANK1; rank--)
	{
		std::cout <<rank +1<< " ";
		for (tFile file = FILEA ; file <= FILEH; file++)
		{
			std::cout << boardc[getSquare(file,rank)];
		}
		std::cout << std::endl;
	}
	std::cout << std::endl << "  abcdefgh" << sync_endl;
}


