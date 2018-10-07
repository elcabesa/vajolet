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

#ifndef SCORE_H_
#define SCORE_H_

#include <cstdint>

//---------------------------------------------
//	typedefs
//---------------------------------------------
typedef int simdScore __attribute__ ((vector_size (16)));
typedef int32_t Score;					/*!< score saved as a single signed int value*/

//---------------------------------------------
//SCORE DEFINITION
//---------------------------------------------

const Score SCORE_NONE = 1700001;
const Score SCORE_INFINITE = 1700000;
const Score SCORE_MATE = 1699990;
const Score SCORE_MATED = -SCORE_MATE;
const Score SCORE_MATE_IN_MAX_PLY = 1690000;
const Score SCORE_KNOWN_WIN = 1500000;
const Score SCORE_MATED_IN_MAX_PLY = -SCORE_MATE_IN_MAX_PLY;

#endif
