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

#ifndef MOVEGEN_H_
#define MOVEGEN_H_

#include <stack>
#include <utility>
#include <cassert>
#include "vajolet.h"
#include "move.h"
#include "position.h"

class Movegen{
private:
	Move moveList[MAX_MOVE_PER_POSITION];
	unsigned int moveListIndex;

public:
	static void initMovegenConstant(void);
	void generateMoves(Position &p);
	inline unsigned int getGeneratedMoveNumber(void){ return moveListIndex;}
	inline Move  & getGeneratedMove(unsigned int x){ return moveList[x];}


	inline static bitMap attackFromRook(tSquare from,bitMap & occupancy){
		assert(from>=0 && from <=squareNumber);
		bitMap res = MG_RANK_ATTACK[from][((occupancy & MG_RANKMASK[from]) >> RANKSHIFT[from])];
		res |= MG_FILE_ATTACK[from][((occupancy & MG_FILEMASK[from])*MG_FILEMAGIC[from]) >> 57];
		return res;
	}
	inline static bitMap attackFromBishop(tSquare from,bitMap & occupancy){
		assert(from>=0 && from <=squareNumber);
		bitMap res= MG_DIAGA8H1_ATTACK[from][((occupancy & MG_DIAGA8H1MASK[from])* MG_DIAGA8H1MAGIC[from])>>57];
		res |= MG_DIAGA1H8_ATTACK[from][((occupancy & MG_DIAGA1H8MASK[from])*MG_DIAGA1H8MAGIC[from]) >> 57];
		return res;
	}

	inline static bitMap attackFromKnight(tSquare from,bitMap occupancy){
		assert(from>=0 && from <=squareNumber);
		return KNIGHT_MOVE[from];
	}
	inline static bitMap attackFromKing(tSquare from,bitMap occupancy){
		assert(from>=0 && from <=squareNumber);
		return KING_MOVE[from];
	}
	inline static bitMap attackFromPawn(tSquare from,unsigned int color ){
		assert(color>=0 && color <=1);
		assert(from>=0 && from <=squareNumber);
		return PAWN_ATTACK[color][from];
	}

	inline static bitMap getRookPseudoAttack(tSquare from){
		return ROOK_PSEUDO_ATTACK[from];
	}

	inline static bitMap getBishopPseudoAttack(tSquare from){
		return BISHOP_PSEUDO_ATTACK[from];
	}

private:
	// Move generator shift for ranks:
	static const int RANKSHIFT[squareNumber];

	static bitMap MG_RANKMASK[squareNumber];
	static bitMap MG_FILEMASK[squareNumber];
	static bitMap MG_DIAGA8H1MASK[squareNumber];
	static bitMap MG_DIAGA1H8MASK[squareNumber];
	static bitMap MG_RANK_ATTACK[squareNumber][64];
	static bitMap MG_FILE_ATTACK[squareNumber][64];
	static bitMap MG_DIAGA8H1_ATTACK[squareNumber][64];
	static bitMap MG_DIAGA1H8_ATTACK[squareNumber][64];
	static bitMap MG_FILEMAGIC[64];
	static bitMap MG_DIAGA8H1MAGIC[64];
	static bitMap MG_DIAGA1H8MAGIC[64];

	// Move generator magic multiplication numbers for files:
	static const bitMap FILEMAGICS[8];
	static const bitMap DIAGA8H1MAGICS[15];
	static const bitMap DIAGA1H8MAGICS[15];
	static bitMap KNIGHT_MOVE[squareNumber];
	static bitMap KING_MOVE[squareNumber];
	static bitMap PAWN_ATTACK[2][squareNumber];
	static bitMap ROOK_PSEUDO_ATTACK[squareNumber];
	static bitMap BISHOP_PSEUDO_ATTACK[squareNumber];

};


#endif /* MOVEGEN_H_ */
