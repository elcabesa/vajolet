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

TBTable::TBTable(const std::string& code, std::string ext, unsigned int sides): _sides(sides), _extension(ext) {
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

TBTable::TBTable(const TBTable& other, std::string ext, unsigned int sides): _sides(sides), _extension(ext) {
	// Use the corresponding WDL table to avoid recalculating all from scratch
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
	
	// Populate entry's PairsData records with data from the just memory mapped file.
	// Called at first access.
	
	const int splitBit = 1;
	const int HasPawnsBit = 2;
	
	const uint8_t* data = &_file + 4;
	
	assert(_hasPawns == !!(*data & HasPawnsBit));
	assert((_key != _key2) == !!(*data & splitBit));
	
	++data;	 // First byte stores flags
	
	const int sides = ((_sides == 2) && (_key != _key2)) ? 2 : 1;
    const tFile maxFile = _hasPawns ? FILED : FILEA;

	// todo rename in a more significant name e.g. pawnsOnBothSides
    const bool pp = hasPawnOnBothSides(); // Pawns on both sides

    assert(!pp || _pawnCount[0]);
	
	for (tFile f = FILEA; f <= maxFile; ++f) {

		/*
		todo a cosa serve? se lo tolgo cambi qualcosa? altrimenti pairdata è non inizializzato?
        for (unsigned int i = 0; i < _sides; ++i) {
			
            *getPairsData(i, f) = PairsData();
		}*/

        int order[][2] = { { *data & 0xF, pp ? *(data + 1) & 0xF : 0xF },
                           { *data >>  4, pp ? *(data + 1) >>  4 : 0xF } };
        data += 1 + pp;

        for (unsigned int k = 0; k < _pieceCount; ++k, ++data) {
            for (unsigned int i = 0; i < _sides; ++i) {
                getPairsData(i, f)->setPiece(k, (i > 0 ? *data >>  4 : *data & 0xF));
			}
		}

		for (int i = 0; i < sides; ++i) {
			getPairsData(i, f)->setGroups(*this, order[i], f);
		}
    }
	
	data += (uintptr_t)data & 1; // Word alignment
	
	for (tFile f = FILEA; f <= maxFile; ++f) {
		for (int i = 0; i < sides; i++) {
			data = getPairsData(i, f)->setSizes(data);
		}
	}
	
	// todo use virtual method?
	// it's only needed for DTZ talbe
	if (getType() == DTZ) {
		setMap(data);
		for (tFile f = FILEA; f <= maxFile; ++f) {
			getPairsData(0, f)->setDtzMap(getMap(), data);
		}
		data += (uintptr_t)data & 1; // Word alignment
	}

}

std::string TBTable::_getCompleteFileName() const {
	return _endgame + "." +  _extension;
	
}

PairsData* TBTable::getPairsData(const unsigned int stm, const tFile f) {
	return &_items[stm % _sides][_hasPawns ? f : 0];
}

bool TBTable::hasPawnOnBothSides() const {
	return _hasPawns && _pawnCount[1];
}

void TBTable::setMap(const uint8_t* x) {
	_map = x;
}

const uint8_t* TBTable::getMap(void) const {
	return _map;
}