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

#include "parameters.h"


EvalParameters::EvalParameters()
{
	_initValues();
}

void EvalParameters::_initValues() {
	for(auto &val: _pieceValue)
	{
		val = simdScore{0,0,0,0};
	}
	_pieceValue[whitePawns] = initialPieceValue[whitePawns];
	_pieceValue[whiteKnights] = initialPieceValue[whiteKnights];
	_pieceValue[whiteBishops] = initialPieceValue[whiteBishops];
	_pieceValue[whiteRooks] = initialPieceValue[whiteRooks];
	_pieceValue[whiteQueens] = initialPieceValue[whiteQueens];
	_pieceValue[whiteKing] = initialPieceValue[whiteKing];

	_pieceValue[blackPawns] = _pieceValue[whitePawns];
	_pieceValue[blackKnights] = _pieceValue[whiteKnights];
	_pieceValue[blackBishops] = _pieceValue[whiteBishops];
	_pieceValue[blackRooks] = _pieceValue[whiteRooks];
	_pieceValue[blackQueens] = _pieceValue[whiteQueens];
	_pieceValue[blackKing] = _pieceValue[whiteKing];
	
	_initPstValues();
}

void EvalParameters::_initPstValues()
{
	/* PST data */
	const int Center[8]	= { -3, -1, +0, +1, +1, +0, -1, -3};
	const int KFile[8]	= { +3, +4, +2, +0, +0, +2, +4, +3};
	const int KRank[8]	= { +1, +0, -2, -3, -4, -5, -6, -7};
	for(bitboardIndex piece = occupiedSquares; piece < lastBitboard; ++piece)
	{
		for(tSquare s = A1; s < squareNumber; ++s)
		{
			assert(s<squareNumber);
			_nonPawnValue[piece] = simdScore{0,0,0,0};
			_pstValue[piece][s] = simdScore{0,0,0,0};
			tRank rank = getRankOf(s);
			tFile file = getFileOf(s);

			if (isValidPiece(piece)) {
				if (!isBlackPiece(piece)) {
					if (isPawn(piece)) {
						_pstValue[piece][s] = simdScore{0,0,0,0};
						if (s == D3) {
							_pstValue[piece][s] = PawnD3;
						} else if (s == D4) {
							_pstValue[piece][s] = PawnD4;
						} else if (s == D5) {
							_pstValue[piece][s] = PawnD5;
						} else if (s == E3) {
							_pstValue[piece][s] = PawnE3;
						} else if (s == E4)	{
							_pstValue[piece][s] = PawnE4;
						} else if (s == E5)	{
							_pstValue[piece][s] = PawnE5;
						}
						_pstValue[piece][s] += PawnRankBonus * static_cast<int>(rank - 2);
						_pstValue[piece][s] += Center[file] * PawnCentering;
					} else if (isKnight(piece)) {
						_pstValue[piece][s] = KnightPST * (Center[file] + Center[rank]);
						if (rank == RANK1) {
							_pstValue[piece][s] -= KnightBackRankOpening;
						}
					} else if (isBishop(piece))	{
						_pstValue[piece][s] = BishopPST * (Center[file] + Center[rank]);
						if (rank == RANK1) {
							_pstValue[piece][s] -= BishopBackRankOpening;
						}
						if (((int)file == (int)rank) || (file + rank == 7)) {
							_pstValue[piece][s] += BishopOnBigDiagonals;
						}
					} else if (isRook(piece)) {
						_pstValue[piece][s] = RookPST * Center[file];
						if (rank == RANK1) {
							_pstValue[piece][s] -= RookBackRankOpening;
						}
					} else if (isQueen(piece)) {
						_pstValue[piece][s] = QueenPST * (Center[file] + Center[rank]);
						if (rank == RANK1) {
							_pstValue[piece][s] -= QueenBackRankOpening;
						}
					} else if (isKing(piece)) {
						_pstValue[piece][s] = simdScore{
								(KFile[file]+KRank[rank]) * KingPST[0],
								(Center[file]+Center[rank]) * KingPST[1],
								0,0};
					}
					// add piece value to pst
					if(!isKing( piece ) )
					{
						_pstValue[piece][s] += _pieceValue[piece];
					}

					if (!isPawn(piece) && !isKing(piece)) {
						_nonPawnValue[piece][0] = _pieceValue[piece][0];
						_nonPawnValue[piece][1] = _pieceValue[piece][1];
					}

				} else {
					tRank r = getRelativeRankOf(s, black);
					tFile f = file;
					_pstValue[piece][s] = -_pstValue[piece - separationBitmap][getSquare(f, r)];

					if(!isPawn(piece) && !isKing(piece)) {
						_nonPawnValue[piece][2] = _pieceValue[piece][0];
						_nonPawnValue[piece][3] = _pieceValue[piece][1];
					}
				}
			} else {
				_pstValue[piece][s] = simdScore{0,0,0,0};
			}
		}
	}

}

SearchParameters::SearchParameters():
razorMargin(20000),
razorMarginDepth(0),
razorMarginCut(0),
razorDepth(4 * 16),
razorReturn(false),

staticNullMovePruningDepth(8 * 16),
staticNullMovePruningValue(6000 / 16),
staticNullMovePruningImprovingBonus(2000),

nullMovePruningDepth(16),
nullMovePruningReduction(3 * 16),
nullMovePruningBonusDepth(0.25),
nullMovePruningBonusThreshold(10000),
nullMovePruningBonusAdditionalRed(16),
nullMovePruningVerificationDepth(12*16),

probCutDepth(5 * 16),
probCutDelta(8000),
probCutDepthRed(3 * 16),

iidDepthPv(5 * 16),
iidDepthNonPv(8 * 16),
iidStaticEvalBonus(10000),
iidDepthRed(2 * 16),
iidDepthRedFactor(4),

singularExpressionPVDepth(6 * 16),
singularExpressionNonPVDepth(8 * 16),
singularExpressionTtDepth(3 * 16)

{}
