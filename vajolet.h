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

#ifndef VAJOLET_H_
#define VAJOLET_H_


// disable assert define
//#define NDEBUG
#include <cassert>

//---------------------------------------------
//	include
//---------------------------------------------

//---------------------------------------------
//	configuration defines
//---------------------------------------------

#define PROGRAM_NAME	"Vajolet2"
#define VERSION			"2.7.0"
#define PRE_RELEASE		"-PRE-8"


//---------------------------------------------
//	define
//---------------------------------------------
//#define DEBUG_EVAL_SIMMETRY
//#define DISABLE_TIME_DIPENDENT_OUTPUT
//#define ENABLE_CHECK_CONSISTENCY


#define MAX_MOVE_PER_POSITION (250)
#define MAX_BAD_MOVE_PER_POSITION (32)
#define ONE_PLY		(16)
#define ONE_PLY_SHIFT (4)
#define STATE_INFO_LENGTH (800)

//---------------------------------------------
//SCORE DEFINITION
//---------------------------------------------

#define SCORE_NONE				(3000001)//(2000000001)
#define SCORE_INFINITE			(3000000)//(2000000000)
#define SCORE_MATE				(2999990)//(1999999990)
#define SCORE_MATED				(-SCORE_MATE)
#define SCORE_MATE_IN_MAX_PLY	(2990000)//(1999990000)
#define SCORE_KNOWN_WIN			(2000000)//(1000000000)
#define SCORE_MATED_IN_MAX_PLY	(-SCORE_MATE_IN_MAX_PLY)
#define SCORE_MAX_OUTPUT_VALUE	(360000)
#define SCORE_MIN_OUTPUT_VALUE	(-SCORE_MAX_OUTPUT_VALUE)

//---------------------------------------------
//
//---------------------------------------------




#define ENABLE_SAFE_OPERATORS_ON(T)                                         \
inline T operator+(const T d1, const T d2) { return T(int(d1) + int(d2)); } \
inline T operator-(const T d1, const T d2) { return T(int(d1) - int(d2)); } \
inline T operator*(int i, const T d) { return T(i * int(d)); }              \
inline T operator*(const T d, int i) { return T(int(d) * i); }              \
inline T operator-(const T d) { return T(-int(d)); }                        \
inline T& operator+=(T& d1, const T d2) { d1 = d1 + d2; return d1; }        \
inline T& operator-=(T& d1, const T d2) { d1 = d1 - d2; return d1; }        \
inline T& operator*=(T& d, int i) { d = T(int(d) * i); return d; }

#define ENABLE_OPERATORS_ON(T) ENABLE_SAFE_OPERATORS_ON(T)                  \
inline T operator++(T& d, int) { d = T(int(d) + 1); return d; }             \
inline T operator--(T& d, int) { d = T(int(d) - 1); return d; }             \
inline T operator/(const T d, int i) { return T(int(d) / i); }              \
inline T& operator/=(T& d, int i) { d = T(int(d) / i); return d; }



//---------------------------------------------
//	typedefs
//---------------------------------------------

typedef unsigned long long bitMap;			/*!< 64 bit bitMap*/
typedef unsigned long long U64;				/*!< 64 bit variable*/
enum tSquare								/*!< square name and directions*/
{
	A1,	B1,	C1,	D1,	E1,	F1,	G1,	H1,
	A2,	B2,	C2,	D2,	E2,	F2,	G2,	H2,
	A3,	B3,	C3,	D3,	E3,	F3,	G3,	H3,
	A4,	B4,	C4,	D4,	E4,	F4,	G4,	H4,
	A5,	B5,	C5,	D5,	E5,	F5,	G5,	H5,
	A6,	B6,	C6,	D6,	E6,	F6,	G6,	H6,
	A7,	B7,	C7,	D7,	E7,	F7,	G7,	H7,
	A8,	B8,	C8,	D8,	E8,	F8,	G8,	H8,
	squareNone,
	squareNumber=64,
	north=8,
	sud=-8,
	est=1,
	ovest=-1,
	square0=0
};

ENABLE_OPERATORS_ON(tSquare)

typedef int simdScore __attribute__ ((vector_size (16)));
typedef signed int Score;					/*!< score saved as a single signed int value*/





#endif /* VAJOLET_H_ */
