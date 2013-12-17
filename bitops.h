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

#include <cpuid.h>
#include "vajolet.h"


//-----------------------------------------------------------------------------
//	inline function
//-----------------------------------------------------------------------------

/*	\brief count the number of 1 bits in a 64 bit variable
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
#ifdef HW_BITCOUNT
static inline unsigned int bitCnt(bitMap bitmap)
{
#if __x86_64__
	return __builtin_popcountll(bitmap);
#else
	return __builtin_popcountl((unsigned int)bitmap)+__builtin_popcountl(bitmap>>32);
#endif
}

#else

static inline unsigned int bitCnt(bitMap bitmap)
{
	int n=0;
	while(bitmap){
		n++;
		bitmap &= bitmap - 1; // reset LS1B

	}
	return n;
}

#endif


/*	\brief get the index of the rightmost one bit
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
static inline tSquare firstOne(bitMap bitmap)
{
#if __x86_64__
	return __builtin_ctzll(bitmap);
#else
	assert(bitmap);

	return ((unsigned long)bitmap)?((tSquare)__builtin_ctzl((unsigned long) bitmap)):((tSquare)__builtin_ctzl(bitmap>>32))+(tSquare)32;
#endif
}


/*	\brief return true if the bitmap has more than one bit set
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline bool moreThanOneBit(bitMap b) {
  return b & (b - 1);
}

//-----------------------------------------------------------------------------
//	function prototype
//-----------------------------------------------------------------------------
void displayBitmap(bitMap b);


#endif /* BITOPS_H_ */
