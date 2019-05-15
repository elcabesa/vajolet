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

#ifndef TBLR_H
#define TBLR_H

#include "tbtypes.h"

class LR {
private:
	uint8_t _lr[3];	// The first 12 bits is the left-hand symbol, the second 12
									// bits is the right-hand symbol. If symbol has length 1,
									// then the left-hand symbol is the stored value.
public:	
	Sym getLeft() { return ((_lr[1] & 0xF) << 8) | _lr[0]; }
	Sym getRight() { return (_lr[2] << 4) | (_lr[1] >> 4); }
	
	LR() = delete;
	~LR() = delete;
	LR(const LR& other) = delete;
	LR(LR&& other) noexcept = delete;
	LR& operator=(const LR& other) = delete;
	LR& operator=(LR&& other) noexcept = delete;
};

static_assert(sizeof(LR) == 3, "LR tree entry must be 3 bytes");

#endif
