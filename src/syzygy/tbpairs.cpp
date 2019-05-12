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
#include <cassert>
#include "tbpairs.h"

bitboardIndex PairsData::_tbPieceConvert(uint8_t rawData) {
	
	static const bitboardIndex map[] = { 
		empty,
		whitePawns,
		whiteKnights,
		whiteBishops,
		whiteRooks,
		whiteQueens,
		whiteKing,
		empty,
		empty,
		blackPawns,
		blackKnights,
		blackBishops,
		blackRooks,
		blackQueens,
		blackKing,
		empty
	};
	return map[rawData];
}

void PairsData::setPiece(unsigned int idx, uint8_t rawData) {
	assert(idx < TBPIECES);
	_pieces[idx] = _tbPieceConvert(rawData);
}

bitboardIndex PairsData::getPiece(unsigned int idx) const {
	assert(idx < TBPIECES);
	return _pieces[idx];
}
