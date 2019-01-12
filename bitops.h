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

#ifndef BITOPS_H_
#define BITOPS_H_

#include <cstdint>

#include "tSquare.h"

using bitMap = uint64_t; /*!< 64 bit bitMap*/

//-----------------------------------------------------------------------------
//	inline function
//-----------------------------------------------------------------------------

/*	\brief count the number of 1 bits in a 64 bit variable
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static inline unsigned int bitCnt(const bitMap b)
{
	return __builtin_popcountll(b);
}


/*	\brief get the index of the rightmost one bit
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
static inline tSquare firstOne(const bitMap b)
{
	return (tSquare)__builtin_ctzll(b);
}

static inline tSquare iterateBit(bitMap& b)
{
	const tSquare t = firstOne(b);
	b &= ( b - 1 );
	return t;

}


/*	\brief return true if the bitmap has more than one bit set
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool moreThanOneBit(const bitMap b)
{
  return b & (b - 1);
}

//-----------------------------------------------------------------------------
//	function prototype
//-----------------------------------------------------------------------------
void displayBitmap(const bitMap b);


#endif /* BITOPS_H_ */
