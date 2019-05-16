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

#include <iostream>

#include <cassert>
#include <vector>

#include "data.h"
#include "movegen.h"
#include "tbCommonData.h"

unsigned int TBCommonData::_MapB1H1H7[squareNumber] = {0};
unsigned int TBCommonData::_MapA1D1D4[squareNumber] = {0};
unsigned int TBCommonData::_MapKK[10][squareNumber] = {0};
unsigned int TBCommonData::_Binomial[6][squareNumber] = {0};
tSquare TBCommonData::_MapPawns[squareNumber] = {A1};
unsigned int TBCommonData::_LeadPawnIdx[6][squareNumber] = {0};
unsigned int TBCommonData::_LeadPawnsSize[6][4] = {0};

int TBCommonData::_offsetA1H8(const tSquare sq) {
	return int(getRankOf(sq)) - int(getFileOf(sq));
}

bool TBCommonData::isOnDiagonalA1H8(const tSquare sq) {
	return (0 == _offsetA1H8(sq));
}
bool TBCommonData::isAboveDiagonalA1H8(const tSquare sq) {
	return (_offsetA1H8(sq) > 0);
}
bool TBCommonData::isBelowDiagonalA1H8(const tSquare sq) {
	return (_offsetA1H8(sq) < 0);
}

void TBCommonData::_initMapB1H1H7() {
	// MapB1H1H7[] encodes a square below a1-h8 diagonal to 0..27
	unsigned int code = 0;
	for (tSquare s = A1; s < squareNumber; ++s) {
		if (isBelowDiagonalA1H8(s)) {
			_MapB1H1H7[s] = code++;
		}
	}
}

void TBCommonData::_initMapA1D1D4() {
	// MapA1D1D4[] encodes a square in the a1-d1-d4 triangle to 0..9
	std::vector<tSquare> diagonal;
	unsigned int code = 0;
	for (tSquare s = A1; s <= D4; ++s) {
		if (isBelowDiagonalA1H8(s) && getFileOf(s) <= FILED) {
			_MapA1D1D4[s] = code++;
		}	else if (isOnDiagonalA1H8(s) && getFileOf(s) <= FILED) {
			diagonal.push_back(s);
		}
	}

	// Diagonal squares are encoded as last ones
	for (auto s : diagonal) {
		_MapA1D1D4[s] = code++;
	}
}

void TBCommonData::_initMapKK() {
	// MapKK[] encodes all the 461 possible legal positions of two kings where
	// the first is in the a1-d1-d4 triangle. If the first king is on the a1-d4
	// diagonal, the other one shall not to be above the a1-h8 diagonal.
	std::vector<std::pair<unsigned int, tSquare>> bothOnDiagonal;
	unsigned int code = 0;
	for (unsigned int idx = 0; idx < 10; ++idx) {
		for (tSquare s1 = A1; s1 <= D4; ++s1) {
			if (getMapA1D1D4(s1) == idx && ((idx != 0) || s1 == B1)) { // SQ_B1 is mapped to 0
				for (tSquare s2 = A1; s2 < squareNumber; ++s2) {
					if (isSquareSet((Movegen::attackFrom<whiteKing>(s1) | bitSet(s1)), s2)) {
						continue; // Illegal position
					}	else if (isOnDiagonalA1H8(s1) && isAboveDiagonalA1H8(s2)) {
						continue; // First on diagonal, second above
					}	else if (isOnDiagonalA1H8(s1) && isOnDiagonalA1H8(s2)) {
						bothOnDiagonal.emplace_back(idx, s2);
					} else {
						_MapKK[idx][s2] = code++;
					}
				}
			}
		}
	}

	// Legal positions with both kings on diagonal are encoded as last ones
	for (auto p : bothOnDiagonal) {
		_MapKK[p.first][p.second] = code++;
	}
}

void TBCommonData::_initBinomial() {
	// Binomial[] stores the Binomial Coefficents using Pascal rule. There
	// are Binomial[k][n] ways to choose k elements from a set of n elements.
	_Binomial[0][0] = 1;

	for (tSquare n = B1; n < squareNumber; ++n) { // Squares
		for (int k = 0; k < 6 && k <= n; ++k) {// Pieces
			_Binomial[k][n] = (k > 0 ? _Binomial[k - 1][n - 1] : 0)
		                    + (k < n ? _Binomial[k    ][n - 1] : 0);
		}
	}
}

void TBCommonData::_initPawnsData() {
	// MapPawns[s] encodes squares a2-h7 to 0..47. This is the number of possible
	// available squares when the leading one is in 's'. Moreover the pawn with
	// highest MapPawns[] is the leading pawn, the one nearest the edge and,
	// among pawns with same file, the one with lowest rank.
	tSquare availableSquares = H6; // Available squares when lead pawn is in a2

	// Init the tables for the encoding of leading pawns group: with 7-men TB we
	// can have up to 5 leading pawns (KPPPPPK).
	for (int leadPawnsCnt = 1; leadPawnsCnt <= 5; ++leadPawnsCnt) {
		for (tFile f = FILEA; f <= FILED; ++f) {
			// Restart the index at every file because TB table is splitted
			// by file, so we can reuse the same index for different files.
			int idx = 0;

			// Sum all possible combinations for a given file, starting with
			// the leading pawn on rank 2 and increasing the rank.
			for (tRank r = RANK2; r <= RANK7; ++r) {
				tSquare sq = getSquare(f, r);

				// Compute MapPawns[] at first pass.
				// If sq is the leading pawn square, any other pawn cannot be
				// below or more toward the edge of sq. There are 47 available
				// squares when sq = a2 and reduced by 2 for any rank increase
				// due to mirroring: sq == a3 -> no a2, h2, so MapPawns[a3] = 45
				if (leadPawnsCnt == 1) {
						_MapPawns[sq] = availableSquares;
						--availableSquares;
						_MapPawns[sq ^ 7] = availableSquares; // Horizontal flip
						--availableSquares;
				}
				_LeadPawnIdx[leadPawnsCnt][sq] = idx;
				idx += getBinomial(leadPawnsCnt - 1, getMapPawns(sq));
			}
			// After a file is traversed, store the cumulated per-file index
			_LeadPawnsSize[leadPawnsCnt][f] = idx;
		}
	}
}

unsigned int TBCommonData::getMapB1H1H7(const tSquare sq) {
	assert(sq < squareNumber);
	return _MapB1H1H7[sq];
}

unsigned int TBCommonData::getMapA1D1D4(const tSquare sq) {
	assert(sq < squareNumber);
	return _MapA1D1D4[sq];
}

unsigned int TBCommonData::getMapKK(const tSquare sq1, const tSquare sq2) {
	assert(sq1 < squareNumber);
	assert(sq2 < squareNumber);
	assert(getMapA1D1D4(sq1) < 10);
	return _MapKK[getMapA1D1D4(sq1)][sq2];
}

unsigned int TBCommonData::getBinomial(const unsigned int idx, const tSquare sq) {
	assert(idx < 6);
	assert(sq < squareNumber);
	return _Binomial[idx][sq];
}

tSquare TBCommonData::getMapPawns(const tSquare sq) {
	assert(sq < squareNumber);
	return _MapPawns[sq];
}

unsigned int TBCommonData::getLeadPawnIdx(const unsigned int idx, const tSquare sq) {
	assert(idx <6);
	assert(sq < squareNumber);
	return _LeadPawnIdx[idx][sq];
}

unsigned int TBCommonData::getLeadPawnsSize(const unsigned int idx, const tFile file) {
	assert(idx <6);
	assert(file < FILEE);
	return _LeadPawnsSize[idx][file];
}

void TBCommonData::init() {
	_initMapB1H1H7();
	_initMapA1D1D4();
	_initMapKK();
	_initBinomial();
	_initPawnsData();
}