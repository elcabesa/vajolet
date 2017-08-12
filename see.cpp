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

#include "vajolet.h"

#include "io.h"
#include "move.h"
#include "position.h"
#include "movegen.h"
#include "bitops.h"


Score Position::seeSign(const Move& m) const
{
	assert(m.packed);
	if (pieceValue[getPieceAt((tSquare)m.bit.from)][0] <= pieceValue[getPieceAt((tSquare)m.bit.to)][0])
	{
		return 1;
	}

	return see(m);
}





Score Position::see(const Move& m) const
{

	assert(m.packed);

	tSquare from = (tSquare)m.bit.from, to = (tSquare)m.bit.to;
	const int relativeRank = getNextTurn() ? 7-RANKS[to] : RANKS[to];
	bitMap occupied = getOccupationBitmap() ^ bitSet(from);
	eNextMove color = getPieceAt(from) > separationBitmap ? blackTurn : whiteTurn;

	Score swapList[64];
	unsigned int slIndex = 1;
	bitMap colorAttackers;
	bitboardIndex captured;

	swapList[0] = pieceValue[getPieceAt(to)][0];
	captured = bitboardIndex(getPieceAt(from) % separationBitmap);

	if( isEnPassantMove(m) )
	{
		occupied ^= bitSet(to - pawnPush(color));
		swapList[0] = pieceValue[whitePawns][0];
	}
	if( isCastleMove(m) )
	{
		return 0;
	}
	if( isPromotionMove(m) )
	{
		captured = bitboardIndex(whiteQueens + m.bit.promotion);
		swapList[0] += pieceValue[whiteQueens + m.bit.promotion][0] - pieceValue[whitePawns][0];
	}

	// Find all attackers to the destination square, with the moving piece
	// removed, but possibly an X-ray attacker added behind it.
	bitMap && attackers = getAttackersTo(to, occupied) & occupied;

	// If the opponent has no attackers we are finished
	color = (eNextMove)(blackTurn - color);
	assert(Pieces + color < lastBitboard);
	colorAttackers = attackers & getBitmap((bitboardIndex)(Pieces + color));


	if (!colorAttackers)
	{
		return swapList[0];
	}


	// The destination square is defended, which makes things rather more
	// difficult to compute. We proceed by building up a "swap list" containing
	// the material gain or loss at each stop in a sequence of captures to the
	// destination square, where the sides alternately capture, and always
	// capture with the least valuable piece. After each capture, we look for
	// new X-ray attacks from behind the capturing piece.

	assert(captured<lastBitboard);
	assert(getPieceAt(from) != empty);
	do
	{
		assert(slIndex < 64);

		// Add the new entry to the swap list
		swapList[slIndex] = -swapList[slIndex - 1] + pieceValue[captured][0];
		slIndex++;

		// Locate and remove the next least valuable attacker
		bitboardIndex nextAttacker = (bitboardIndex)(Pawns);

		while(nextAttacker >= King)
		{
			bitMap att = getBitmap(bitboardIndex(nextAttacker + color)) & colorAttackers;

			if(att)
			{
				att= att & ~(att - 1); // find only one attacker
				occupied ^= att;
				attackers ^= att;

				if (nextAttacker == Pawns || nextAttacker == Bishops || nextAttacker == Queens){
					attackers |= Movegen::attackFrom<Position::whiteBishops>(to,occupied)& (getBitmap(whiteBishops) |getBitmap(blackBishops) |getBitmap(whiteQueens) |getBitmap(blackQueens));
				}

				if (nextAttacker == Rooks || nextAttacker == Queens){
					assert(to<squareNumber);
					attackers |= Movegen::attackFrom<Position::whiteRooks>(to,occupied)& (getBitmap(whiteRooks) |getBitmap(blackRooks) |getBitmap(whiteQueens) |getBitmap(blackQueens));
				}
				attackers &= occupied;
				captured = nextAttacker;
				if(relativeRank == 7 && captured == whitePawns)
				{
					captured = whiteQueens;
				}
				break;
			}
			nextAttacker = bitboardIndex(nextAttacker - 1);
		}


		color = (eNextMove)(blackTurn-color);
		colorAttackers = attackers & getBitmap((bitboardIndex)(Pieces + color));

		// Stop before processing a king capture
		if (captured == King && colorAttackers)
		{
			swapList[slIndex++] = pieceValue[whiteKing][0];
			break;
		}

	}while(colorAttackers);

	// Having built the swap list, we negamax through it to find the best
	// achievable score from the point of view of the side to move.
	while (--slIndex)
	{
		swapList[slIndex - 1] = std::min(-swapList[slIndex], swapList[slIndex-1]);
	}

	return swapList[0];

}
