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
#include <list>
#include <utility>
#include "vajolet.h"
#include "move.h"
#include "position.h"

#define KING_SIDE_CASTLE (0)
#define QUEEN_SIDE_CASTLE (1)
class Movegen{
	static Move NOMOVE;
public:
	enum genType{
		captureMg,			// generate capture moves
		quietMg,			// generate quiet moves
		quietChecksMg,		// generate quiet moves giving check
		allNonEvasionMg,	// generate all moves while not in check
		allEvasionMg,		// generate all moves while in check
		allMg				// general generate all move

	};

	enum eStagedGeneratorState{
		getTT,
		generateCaptureMoves,
		iterateCaptureMoves,
		generateQuietMoves,
		iterateQuietMoves,
		finishedNormalStage,

		getTTevasion,
		generateEvasionMoves,
		iterateEvasionMoves,
		finishedEvasionStage,

		getQsearchTT,
		generateQuiescentMoves,
		iterateQuiescentMoves,
		finishedQuiescentStage,

		getQsearchTTquiet,
		generateQuiescentCaptures,
		iterateQuiescentCaptures,
		generateQuietCheks,
		iterateQuietcChecks,
		finishedQuiescentQuietStage,

	}stagedGeneratorState;
private:
	Move moveList[MAX_MOVE_PER_POSITION];
	unsigned int moveListSize;
	unsigned int moveListPosition;

	const Position & pos;
	Move ttMove;


public:
	Movegen(const Position & p, Move & m): pos(p)
	{
		ttMove.packed=m.packed;
		Position::state &s =pos.getActualState();
		if(s.checkers){
			stagedGeneratorState=getTTevasion;
		}
		else{
			stagedGeneratorState=getTT;
		}
		moveListPosition=0;
		moveListSize=0;
	}
	int setupQuiescentSearch(bool checkers,int depth){
		if(checkers){
			stagedGeneratorState=getTTevasion;
			return -1*ONE_PLY;
		}else{
			if(depth>-1*ONE_PLY){
				stagedGeneratorState=getQsearchTTquiet;
				return -1*ONE_PLY;
			}else{
				stagedGeneratorState=getQsearchTT;
				if(!pos.isCaptureMove(ttMove)){
					ttMove.packed=0;
				}
				return -2*ONE_PLY;
			}

		}
	}

	static void initMovegenConstant(void);
	template<Movegen::genType type>	void generateMoves();
	bool isMoveLegal(const Position&p, Move &m) const;
	inline unsigned int getGeneratedMoveNumber(void){ return moveListSize;}

	Move & getNextMove(void);


	inline static bitMap attackFrom(Position::bitboardIndex piece,tSquare from,bitMap & occupancy){
		switch(piece){
		case Position::whiteKing:
		case Position::blackKing:
			return attackFromKing(from);
			break;
		case Position::whiteQueens:
		case Position::blackQueens:
			return attackFromRook(from,occupancy) | attackFromBishop(from,occupancy);
			break;
		case Position::whiteRooks:
		case Position::blackRooks:
			return attackFromRook(from,occupancy);
			break;
		case Position::whiteBishops:
		case Position::blackBishops:
			return attackFromBishop(from,occupancy);
			break;
		case Position::whiteKnights:
		case Position::blackKnights:
			return attackFromKnight(from);
			break;
		case Position::whitePawns:
			return attackFromPawn(from,0);
			break;
		case Position::blackPawns:
			return attackFromPawn(from,1);
			break;
		default:
			return 0;

		}
	}

	inline static bitMap attackFromRook(tSquare from,const bitMap & occupancy){
		assert(from <=squareNumber);
		bitMap res = MG_RANK_ATTACK[from][((occupancy & MG_RANKMASK[from]) >> RANKSHIFT[from])];
		res |= MG_FILE_ATTACK[from][((occupancy & MG_FILEMASK[from])*MG_FILEMAGIC[from]) >> 57];
		return res;
	}
	inline static bitMap attackFromBishop(tSquare from,const bitMap & occupancy){
		assert(from <=squareNumber);
		bitMap res= MG_DIAGA8H1_ATTACK[from][((occupancy & MG_DIAGA8H1MASK[from])* MG_DIAGA8H1MAGIC[from])>>57];
		res |= MG_DIAGA1H8_ATTACK[from][((occupancy & MG_DIAGA1H8MASK[from])*MG_DIAGA1H8MAGIC[from]) >> 57];
		return res;
	}

	inline static bitMap attackFromKnight(tSquare from){
		assert(from <=squareNumber);
		return KNIGHT_MOVE[from];
	}
	inline static bitMap attackFromKing(tSquare from){
		assert(from <=squareNumber);
		return KING_MOVE[from];
	}
	inline static bitMap attackFromPawn(tSquare from,unsigned int color ){
		assert(color <=1);
		assert(from <=squareNumber);
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
	static bitMap castlePath[2][2];

	/*inline void swapMoves(Move * list, unsigned int index1, unsigned int index2){
		Move temp;
		temp.packed=list[index1].packed;
		moveList[index1].packed=moveList[index2].packed;
		moveList[index2].packed=temp.packed;
	}*/

};


#endif /* MOVEGEN_H_ */
