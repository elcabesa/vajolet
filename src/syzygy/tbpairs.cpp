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
#include "tbtable.h"
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

// Group together pieces that will be encoded together. The general rule is that
// a group contains pieces of same type and color. The exception is the leading
// group that, in case of positions withouth pawns, can be formed by 3 different
// pieces (default) or by the king pair when there is not a unique piece apart
// from the kings. When there are pawns, pawns are always first in pieces[].
//
// As example KRKN -> KRK + N, KNNK -> KK + NN, KPPKP -> P + PP + K + K
//
// The actual grouping depends on the TB generator and can be inferred from the
// sequence of pieces in piece[] array.
void PairsData::_setGroups(const TBTable& tbt, const int order[], const tFile f) {
	int n = 0, firstLen = tbt.hasPawns() ? 0 : tbt.hasUniquePieces() ? 3 : 2;
	_groupLen[n] = 1;

	// Number of pieces per group is stored in groupLen[], for instance in KRKN
	// the encoder will default on '111', so groupLen[] will be (3, 1).
	for (unsigned int i = 1; i < tbt.getPieceCount(); ++i)
		if (--firstLen > 0 || _pieces[i] == _pieces[i - 1]) {
			_groupLen[n]++;
		} else {
			_groupLen[++n] = 1;
		}

	_groupLen[++n] = 0; // Zero-terminated

	// The sequence in pieces[] defines the groups, but not the order in which
	// they are encoded. If the pieces in a group g can be combined on the board
	// in N(g) different ways, then the position encoding will be of the form:
	//
	//           g1 * N(g2) * N(g3) + g2 * N(g3) + g3
	//
	// This ensures unique encoding for the whole position. The order of the
	// groups is a per-table parameter and could not follow the canonical leading
	// pawns/pieces -> remainig pawns -> remaining pieces. In particular the
	// first group is at order[0] position and the remaining pawns, when present,
	// are at order[1] position.
	bool pp = tbt.hasPawnOnBothSides(); // Pawns on both sides
	int next = pp ? 2 : 1;
	int freeSquares = 64 - _groupLen[0] - (pp ? _groupLen[1] : 0);
	uint64_t idx = 1;

	for (int k = 0; next < n || k == order[0] || k == order[1]; ++k)
		if (k == order[0]) // Leading pawns or pieces
		{
			_groupIdx[0] = idx;
			idx *= tbt.hasPawns() ? _LeadPawnsSize[_groupLen[0]][f]
				  : tbt.hasUniquePieces() ? 31332 : 462;
		}
		else if (k == order[1]) // Remaining pawns
		{
			_groupIdx[1] = idx;
			idx *= Binomial[_groupLen[1]][48 - _groupLen[0]];
		}
		else // Remainig pieces
		{
			_groupIdx[next] = idx;
			idx *= Binomial[_groupLen[next]][freeSquares];
			freeSquares -= _groupLen[next++];
		}

	_groupIdx[n] = idx;
}

void PairsData::initData() {
	
}