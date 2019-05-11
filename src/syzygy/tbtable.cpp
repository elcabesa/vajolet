/*
	This file is part of Vajolet.
	
	Copyright (c) 2013 Ronald de Man
	Copyright (c) 2015 basil00
	Copyright (C) 2016-2019 Marco Costalba, Lucas Braesch
	Modifications Copyright (c) 2016-2019 by Jon Dart
	Modifications Copyright (c) 2019 by Marco Belli

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

#include "eCastle.h"
#include "position.h"

#include "tbtable.h"
#include "tbvalidater.h"

TBTable::TBTable(const std::string& code, std::string ext): _extension(ext) {
	_endgame = code;
	Position pos;
	
	_key = pos.setup(getEndGame(), white).getMaterialKey();
	_pieceCount = pos.getPieceCount(occupiedSquares);
	_hasPawns = pos.getBitmap(whitePawns) | pos.getBitmap(blackPawns);
	
	_hasUniquePieces = false;
	for (bitboardIndex idx = empty; idx < lastBitboard; ++idx) {
		if (isValidPiece(idx) && !isKing(idx)) {
			if (pos.getPieceCount(idx) == 1) {
				_hasUniquePieces = true;
				break;
			}
		}
	}
	
	// Set the leading color. In case both sides have pawns the leading color
	// is the side with less pawns because this leads to better compression.
	bool leadingWhite = 
		!pos.getPieceCount(blackPawns)
		|| ( pos.getPieceCount(whitePawns)
		&& pos.getPieceCount(blackPawns) >= pos.getPieceCount(whitePawns));
		
	_pawnCount[0] = pos.getPieceCount(leadingWhite ? whitePawns : blackPawns);
	_pawnCount[1] = pos.getPieceCount(leadingWhite ? blackPawns : whitePawns);

	_key2 = pos.setup(getEndGame(), black).getMaterialKey();
}

TBTable::TBTable(const TBTable& other, std::string ext): _extension(ext) {
	// Use the corresponding WDL table to avoid recalculating all from scratch
	// todo remember to change file name extension
	_endgame = other._endgame;
	_key = other._key;
	_key2 = other._key2;
	_pieceCount = other._pieceCount;
	_hasPawns = other._hasPawns;
	_hasUniquePieces = other._hasUniquePieces;
	_pawnCount[0] = other._pawnCount[0];
	_pawnCount[1] = other._pawnCount[1];
}

std::string TBTable::getEndGame() const{
	return _endgame;
}

void TBTable::mapFile() {
	std::call_once(_mappedFlag, &TBTable::_mapFile, this);
}

void TBTable::_mapFile() {
	_file = TBFile(_getCompleteFileName());
	TBValidater::validate(_file, getType(), _getCompleteFileName());
	
	const int splitBit = 1;
	const int HasPawnsBit = 2;
	
	const uint8_t* data = &_file +4;

    assert(_hasPawns == !!(*data & HasPawnsBit));
    assert((_key != _key2) == !!(*data & splitBit));
	
	++data;	 // First byte stores flags
	
}

std::string TBTable::_getCompleteFileName() const {
	return _endgame + "." +  _extension;
	
}