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

#include "move.h"
#include "position.h"
#include "movegen.h"
#include "bitops.h"


Score Position::seeSign(Move m) const {
	if (pieceValue[board[m.from]%emptyBitmap][0] <= pieceValue[board[m.to]%emptyBitmap][0])
	{
		return 1;
	}

	return see(m);
}





Score Position::see(Move m) const {

//	display();
//	sync_cout<<displayUci(m)<<sync_endl;

	tSquare from=(tSquare)m.from, to=(tSquare)m.to;
	bitMap occupied=bitBoard[occupiedSquares]^bitSet(from);
//	displayBitMap(occupied);
	eNextMove color=board[from]>emptyBitmap?blackTurn:whiteTurn;

	Score swapList[64];
	unsigned int slIndex = 1;
	bitMap attackers,colorAttackers;
	bitboardIndex captured;

	swapList[0] = pieceValue[board[to]%emptyBitmap][0];

	if(m.flags== Move::fenpassant){
		occupied ^= to- pawnPush(color);
		swapList[0] = pieceValue[whitePawns][0];
	}
	if(m.flags== Move::fpromotion){
		swapList[0] =pieceValue[whiteQueens+m.promotion][0]-pieceValue[whitePawns][0];
	}

	// Find all attackers to the destination square, with the moving piece
	// removed, but possibly an X-ray attacker added behind it.
	attackers = getAttackersTo(to, occupied) & occupied;
//	displayBitMap(attackers);

	// If the opponent has no attackers we are finished
	color = (eNextMove)(blackTurn-color);
	colorAttackers = attackers & bitBoard[Pieces+color];

//	displayBitMap(colorAttackers);

	if (!colorAttackers){
		return swapList[0];
	}


	// The destination square is defended, which makes things rather more
	// difficult to compute. We proceed by building up a "swap list" containing
	// the material gain or loss at each stop in a sequence of captures to the
	// destination square, where the sides alternately capture, and always
	// capture with the least valuable piece. After each capture, we look for
	// new X-ray attacks from behind the capturing piece.
	captured = bitboardIndex(board[from]%emptyBitmap);

	do {
		assert(slIndex < 64);

		// Add the new entry to the swap list
		swapList[slIndex] = -swapList[slIndex - 1] + pieceValue[captured][0];
		if(m.flags== Move::fpromotion){
			swapList[0] =pieceValue[whiteQueens+m.promotion][0]-pieceValue[whitePawns][0];
		}
		slIndex++;

		// Locate and remove the next least valuable attacker
		bitboardIndex nextAttacker= (bitboardIndex)(Pawns);
		while(nextAttacker>=King){
			bitMap att=bitBoard[nextAttacker+color] &colorAttackers;
//			displayBitMap(att);
			if(att){
				att= att & ~(att - 1); // find only one attacker
//				displayBitMap(att);
				occupied ^= att;
//				displayBitMap(occupied);
				attackers ^= att;
//				displayBitMap(attackers);

				if (nextAttacker == Pawns || nextAttacker == Bishops || nextAttacker == Queens){
					attackers |= Movegen::attackFromBishop(to,occupied)& (bitBoard[whiteBishops] |bitBoard[blackBishops] |bitBoard[whiteQueens] |bitBoard[blackQueens]);
				}

				if (nextAttacker == Rooks || nextAttacker == Queens){
					attackers |= Movegen::attackFromRook(to,occupied)& (bitBoard[whiteRooks] |bitBoard[blackRooks] |bitBoard[whiteQueens] |bitBoard[blackQueens]);
				}
				attackers &= occupied;
//				displayBitMap(attackers);

				break;
			}
			nextAttacker=bitboardIndex(nextAttacker-1);
		}
		//captured = min_attacker<PAWN>(byTypeBB, to, stmAttackers, occupied, attackers);


		color = (eNextMove)(blackTurn-color);
		colorAttackers = attackers & bitBoard[Pieces+color];
//		displayBitMap(colorAttackers);

		// Stop before processing a king capture
		if (captured == King && colorAttackers)
		{
			swapList[slIndex++] = pieceValue[whiteKing][0];
			break;
		}

	} while (colorAttackers);


/*	// If we are doing asymmetric SEE evaluation and the same side does the first
	  // and the last capture, he loses a tempo and gain must be at least worth
	  // 'asymmThreshold', otherwise we replace the score with a very low value,
	  // before negamaxing.
	  if (asymmThreshold)
	      for (int i = 0; i < slIndex; i += 2)
	          if (swapList[i] < asymmThreshold)
	              swapList[i] = - QueenValueMg * 16;

*/
	// Having built the swap list, we negamax through it to find the best
	// achievable score from the point of view of the side to move.
	while (--slIndex){
		swapList[slIndex-1] = std::min(-swapList[slIndex], swapList[slIndex-1]);
	}



	return swapList[0];











}
