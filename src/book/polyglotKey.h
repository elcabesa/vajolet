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


#ifndef POLYGLOT_KEY_H_
#define POLYGLOT_KEY_H_

#include <cstdint>
#include "bitBoardIndex.h"

class Position;

class PolyglotKey
{
private:
	const unsigned int _pieceMapping[lastBitboard];
	const union {
		uint64_t PolyGlotRandoms[781];
		struct {
			uint64_t psq[12][64];  // [piece][square]
			uint64_t castle[4];    // [castle right]
			uint64_t enpassant[8]; // [file]
			uint64_t turn;
		} Zobrist;
	}_PG;
public:
	PolyglotKey();
	uint64_t get(const Position& pos);
	
};

#endif