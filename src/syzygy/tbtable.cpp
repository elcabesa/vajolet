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

#include "tbCommonData.h"
#include "tbtable.h"
#include "tbvalidater.h"

TBTable::TBTable(const std::string& code, const std::string& ext, unsigned int sides): _endgame(code), _sides(sides), _map(nullptr), _extension(ext)  {
	Position pos(nullptr, Position::pawnHash::off);
	
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

TBTable::TBTable(const TBTable& other, const std::string& ext, unsigned int sides): _endgame(other._endgame), _key(other._key), _key2(other._key2), _pieceCount(other._pieceCount), _hasPawns(other._hasPawns), _hasUniquePieces(other._hasUniquePieces), _sides(sides), _map(nullptr), _extension(ext) {
	// Use the corresponding WDL table to avoid recalculating all from scratch
	_pawnCount[0] = other._pawnCount[0];
	_pawnCount[1] = other._pawnCount[1];
}

std::string TBTable::getEndGame() const{
	return _endgame;
}

bool TBTable::mapFile() {
	std::call_once(_mappedFlag, &TBTable::_mapFile, this);
	return _file.isValid();
}

// Populate entry's PairsData records with data from the just memory mapped file.
// Called at first access.
void TBTable::_mapFile(){
	if (!TBFile::exist(getCompleteFileName()))
		return;
	
	_file = TBFile(getCompleteFileName());
	
	TBValidater::validate(_file, getType(), getCompleteFileName());
	
	// Populate entry's PairsData records with data from the just memory mapped file.
	// Called at first access.
#ifndef NDEBUG
	const int splitBit = 1;
	const int HasPawnsBit = 2;
#endif
	
	const uint8_t* data = &_file + 4;
	
	assert(_hasPawns == !!(*data & HasPawnsBit));
	assert((_key != _key2) == !!(*data & splitBit));
	
	++data;	 // First byte stores flags
	
	const unsigned int sides = ((_sides == 2) && (_key != _key2)) ? 2 : 1;
    const tFile maxFile = _hasPawns ? FILED : FILEA;

    const bool pawnsOnBothSides = hasPawnOnBothSides(); // Pawns on both sides

    assert(!pawnsOnBothSides || _pawnCount[0]);
	
	for (tFile f = FILEA; f <= maxFile; ++f) {

        int order[][2] = { { *data & 0xF, pawnsOnBothSides ? *(data + 1) & 0xF : 0xF },
                           { *data >>  4, pawnsOnBothSides ? *(data + 1) >>  4 : 0xF } };
        data += 1 + pawnsOnBothSides;

        for (unsigned int k = 0; k < _pieceCount; ++k, ++data) {
            for (unsigned int i = 0; i < sides; ++i) {
                getPairsData(i, f).setPiece(k, (i > 0 ? *data >>  4 : *data & 0xF));
			}
		}

		for (unsigned int i = 0; i < sides; ++i) {
			getPairsData(i, f).setGroups(*this, order[i], f);
		}
    }
	
	data += (uintptr_t)data & 1; // Word alignment
	
	for (tFile f = FILEA; f <= maxFile; ++f) {
		for (unsigned int i = 0; i < sides; i++) {
			data = getPairsData(i, f).setSizes(data);
		}
	}
	// it's only needed for DTZ talbe
	if (getType() == TBType::DTZ) {
		setMap(data);
		for (tFile f = FILEA; f <= maxFile; ++f) {
			data = getPairsData(0, f).setDtzMap(getMap(), data);
		}
		data += (uintptr_t)data & 1; // Word alignment
	}
	
	for (tFile f = FILEA; f <= maxFile; ++f) {
        for (unsigned int i = 0; i < sides; i++) {
            data = getPairsData(i, f).setSparseIndex(data);
        }
	}

    for (tFile f = FILEA; f <= maxFile; ++f) {
        for (unsigned int i = 0; i < sides; i++) {
            data = getPairsData(i, f).setBlockLength(data);
        }
	}

    for (tFile f = FILEA; f <= maxFile; ++f) {
        for (unsigned int i = 0; i < sides; i++) {
            data = getPairsData(i, f).setData(data);
        }
	}

}

std::string TBTable::getCompleteFileName() const {
	return _endgame + "." +  _extension;
	
}

PairsData& TBTable::getPairsData(const unsigned int stm, const tFile f) {
	return _items[stm % _sides][_hasPawns ? f : 0];
}

const PairsData& TBTable::getPairsData(const unsigned int stm, const tFile f) const {
	return _items[stm % _sides][_hasPawns ? f : 0];
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

// Compute a unique index out of a position and use it to probe the TB file. To
// encode k pieces of same type and color, first sort the pieces by square in
// ascending order s1 <= s2 <= ... <= sk then compute the unique index as:
//
//      idx = Binomial[1][s1] + Binomial[2][s2] + ... + Binomial[k][sk]
//
WDLScore TBTable::probe(const Position& pos, WDLScore wdl, ProbeState& result) {
	tSquare squares[TBPIECES];
	bitboardIndex pieces[TBPIECES];
	uint64_t idx;
	int next = 0, size = 0, leadPawnsCnt = 0;
	bitMap b, leadPawns = 0;
	tFile tbFile = FILEA;

	// A given TB entry like KRK has associated two material keys: KRvk and Kvkr.
	// If both sides have the same pieces keys are equal. In this case TB tables
	// only store the 'white to move' case, so if the position to lookup has black
	// to move, we need to switch the color and flip the squares before to lookup.
	bool symmetricBlackToMove = ((getKey() == getKey2()) && pos.isBlackTurn());

	// TB files are calculated for white as stronger side. For instance we have
	// KRvK, not KvKR. A position where stronger side is white will have its
	// material key == entry->key, otherwise we have to switch the color and
	// flip the squares before to lookup.
	bool blackStronger = (pos.getMaterialKey() != getKey());

	int flipColor   = (symmetricBlackToMove || blackStronger) * 8;
	int flipSquares = (symmetricBlackToMove || blackStronger) * 070;
	int stm         = (symmetricBlackToMove || blackStronger) ^ pos.isBlackTurn();

	// For pawns, TB files store 4 separate tables according if leading pawn is on
	// file a, b, c or d after reordering. The leading pawn is the one with maximum
	// MapPawns[] value, that is the one most toward the edges and with lowest rank.
	if (hasPawns()) {

		// In all the 4 tables, pawns are at the beginning of the piece sequence and
		// their color is the reference one. So we just pick the first one.
		bitboardIndex pc = bitboardIndex(getPairsData(0, FILEA).getPiece(0) ^ flipColor);

		assert(isPawn(pc));

		leadPawns = b = pos.getBitmap(pc);
		do {
			squares[size++] = (tSquare)(iterateBit(b) ^ flipSquares);
		} while (b);

		leadPawnsCnt = size;

		std::swap(squares[0], *std::max_element(squares, squares + leadPawnsCnt, TBCommonData::pawnsComp));

		tbFile = getFileOf(squares[0]);
		if (tbFile > FILED) {
			tbFile = getFileOf((tSquare)(squares[0] ^ 7)); // Horizontal flip: SQ_H1 -> SQ_A1
		}
	}

	// DTZ tables are one-sided, i.e. they store positions only for white to
	// move or only for black to move, so check for side to move to be stm,
	// early exit otherwise.
	if (!_checkDtzStm(stm, tbFile)) {
		result = ProbeState::CHANGE_STM;
		return WDLScore::WDLDraw; // don't care, this value is not used
	}

	// Now we are ready to get all the position pieces (but the lead pawns) and
	// directly map them to the correct color and square.
	b = pos.getOccupationBitmap() ^ leadPawns;
	do {
		tSquare s = iterateBit(b);
		squares[size] = (tSquare)(s ^ flipSquares);
		pieces[size++] = bitboardIndex(pos.getPieceAt(s) ^ flipColor);
	} while (b);

	assert(size >= 2);

	const PairsData& d = getPairsData(stm, tbFile);

	// Then we reorder the pieces to have the same sequence as the one stored
	// in pieces[i]: the sequence that ensures the best compression.
	for (int i = leadPawnsCnt; i < size; ++i) {
		for (int j = i; j < size; ++j) {
			if (d.getPiece(i) == pieces[j]) {
				std::swap(pieces[i], pieces[j]);
				std::swap(squares[i], squares[j]);
				break;
			}
		}
	}

	// Now we map again the squares so that the square of the lead piece is in
	// the triangle A1-D1-D4.
	if (getFileOf(squares[0]) > FILED) {
		for (int i = 0; i < size; ++i) {
			squares[i] = (tSquare)(squares[i] ^ 7); // Horizontal flip: SQ_H1 -> SQ_A1
		}
	}

	// Encode leading pawns starting with the one with minimum MapPawns[] and
	// proceeding in ascending order.
	if (hasPawns()) {
		idx = TBCommonData::getLeadPawnIdx(leadPawnsCnt, squares[0]);

		std::sort(squares + 1, squares + leadPawnsCnt, TBCommonData::pawnsComp);

		for (int i = 1; i < leadPawnsCnt; ++i) {
			idx += TBCommonData::getBinomial(i, (tSquare)TBCommonData::getMapPawns(squares[i]));
		}

		goto encode_remaining; // With pawns we have finished special treatments
	}

	// In positions withouth pawns, we further flip the squares to ensure leading
	// piece is below RANK_5.
	if (getRankOf(squares[0]) > RANK4) {
		for (int i = 0; i < size; ++i) {
			squares[i] = (tSquare)(squares[i] ^ 070); // Vertical flip: SQ_A8 -> SQ_A1
		}
	}

	// Look for the first piece of the leading group not on the A1-D4 diagonal
	// and ensure it is mapped below the diagonal.
	for (int i = 0; i < d.getGroupLen(0); ++i) {
		if (TBCommonData::isOnDiagonalA1H8(squares[i])) {
			continue;
		}

		if (TBCommonData::isAboveDiagonalA1H8(squares[i])) {// A1-H8 diagonal flip: SQ_A3 -> SQ_C3
			for (int j = i; j < size; ++j) {
				squares[j] = tSquare(((squares[j] >> 3) | (squares[j] << 3)) & 63);
			}
		}
		break;
	}

	// Encode the leading group.
	//
	// Suppose we have KRvK. Let's say the pieces are on square numbers wK, wR
	// and bK (each 0...63). The simplest way to map this position to an index
	// is like this:
	//
	//   index = wK * 64 * 64 + wR * 64 + bK;
	//
	// But this way the TB is going to have 64*64*64 = 262144 positions, with
	// lots of positions being equivalent (because they are mirrors of each
	// other) and lots of positions being invalid (two pieces on one square,
	// adjacent kings, etc.).
	// Usually the first step is to take the wK and bK together. There are just
	// 462 ways legal and not-mirrored ways to place the wK and bK on the board.
	// Once we have placed the wK and bK, there are 62 squares left for the wR
	// Mapping its square from 0..63 to available squares 0..61 can be done like:
	//
	//   wR -= (wR > wK) + (wR > bK);
	//
	// In words: if wR "comes later" than wK, we deduct 1, and the same if wR
	// "comes later" than bK. In case of two same pieces like KRRvK we want to
	// place the two Rs "together". If we have 62 squares left, we can place two
	// Rs "together" in 62 * 61 / 2 ways (we divide by 2 because rooks can be
	// swapped and still get the same position.)
	//
	// In case we have at least 3 unique pieces (inlcuded kings) we encode them
	// together.
	if (hasUniquePieces()) {

		int adjust1 =  squares[1] > squares[0];
		int adjust2 = (squares[2] > squares[0]) + (squares[2] > squares[1]);

		// First piece is below a1-h8 diagonal. MapA1D1D4[] maps the b1-d1-d3
		// triangle to 0...5. There are 63 squares for second piece and and 62
		// (mapped to 0...61) for the third.
		if (!TBCommonData::isOnDiagonalA1H8(squares[0])) {
			idx = (   TBCommonData::getMapA1D1D4(squares[0])  * 63
				   + (squares[1] - adjust1)) * 62
				   +  squares[2] - adjust2;
		}

		// First piece is on a1-h8 diagonal, second below: map this occurence to
		// 6 to differentiate from the above case, rank_of() maps a1-d4 diagonal
		// to 0...3 and finally MapB1H1H7[] maps the b1-h1-h7 triangle to 0..27.
		else if (!TBCommonData::isOnDiagonalA1H8(squares[1])) {
			idx = (  6 * 63 + getRankOf(squares[0]) * 28
				   + TBCommonData::getMapB1H1H7(squares[1]))       * 62
				   + squares[2] - adjust2;
		}

		// First two pieces are on a1-h8 diagonal, third below
		else if (!TBCommonData::isOnDiagonalA1H8(squares[2])) {
			idx =  6 * 63 * 62 + 4 * 28 * 62
				 +  getRankOf(squares[0])        * 7 * 28
				 + (getRankOf(squares[1]) - adjust1) * 28
				 +  TBCommonData::getMapB1H1H7(squares[2]);
		}

		// All 3 pieces on the diagonal a1-h8
		else {
			idx = 6 * 63 * 62 + 4 * 28 * 62 + 4 * 7 * 28
				 +  getRankOf(squares[0])         * 7 * 6
				 + (getRankOf(squares[1]) - adjust1)  * 6
				 + (getRankOf(squares[2]) - adjust2);
		}
	} else {
		// We don't have at least 3 unique pieces, like in KRRvKBB, just map
		// the kings.
		idx = TBCommonData::getMapKK(squares[0], squares[1]);
	}

encode_remaining:
	idx *= d.getGroupIdx(0);
	tSquare* groupSq = squares + d.getGroupLen(0);

	// Encode remainig pawns then pieces according to square, in ascending order
	bool remainingPawns = hasPawns() && getPawnCount(1);

	while (d.getGroupLen(++next))
	{
		std::sort(groupSq, groupSq + d.getGroupLen(next));
		uint64_t n = 0;

		// Map down a square if "comes later" than a square in the previous
		// groups (similar to what done earlier for leading group pieces).
		for (int i = 0; i < d.getGroupLen(next); ++i)
		{
			auto f = [&](tSquare s) { return groupSq[i] > s; };
			auto adjust = std::count_if(squares, groupSq, f);
			n += TBCommonData::getBinomial(i + 1, (tSquare)(groupSq[i] - adjust - 8 * remainingPawns));
		}

		remainingPawns = false;
		idx += n * d.getGroupIdx(next);
		groupSq += d.getGroupLen(next);
	}

	// Now that we have the index, decompress the pair and get the score
	return _mapScore(tbFile, d.decompress(idx), wdl);
}
