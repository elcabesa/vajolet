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
#include "history.h"

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
		iterateGoodCaptureMoves,
		getKillers,
		generateQuietMoves,
		iterateQuietMoves,
		iterateBadCaptureMoves,
		finishedNormalStage,

		getTTevasion,
		generateEvasionMoves,
		iterateEvasionMoves,
		finishedEvasionStage,

		getQsearchTT,
		generateQuiescentMoves,
		iterateQuiescentMoves,
		finishedQuiescentStage,

		getProbCutTT,
		generateProbCutCaptures,
		iterateProbCutCaptures,
		finishedProbCutStage,

		getQsearchTTquiet,
		generateQuiescentCaptures,
		iterateQuiescentCaptures,
		generateQuietCheks,
		iterateQuietcChecks,
		finishedQuiescentQuietStage,

	}stagedGeneratorState;
private:
	extMove moveList[MAX_MOVE_PER_POSITION];
	extMove badCaptureList[MAX_MOVE_PER_POSITION];
	unsigned int moveListSize;
	unsigned int moveListPosition;

	unsigned int badCaptureSize;
	unsigned int badCapturePosition;
	unsigned int killerPos;
	Score captureThreshold;


	const Position & pos;
	Move ttMove;


public:
	Movegen(const Position & p, Move & ttm): pos(p)
	{
		ttMove=ttm;
		Position::state &s =pos.getActualState();
		if(s.checkers){
			stagedGeneratorState=getTTevasion;
		}
		else{
			stagedGeneratorState=getTT;
		}
		moveListPosition=0;
		moveListSize=0;
		badCaptureSize=0;
		badCapturePosition=0;
		killerPos=0;
		captureThreshold=0;
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
				if(ttMove.packed && !pos.isCaptureMove(ttMove)){
					ttMove=0;
				}
				return -2*ONE_PLY;
			}

		}
	}

	void setupProbCutSearch(Position::bitboardIndex capturePiece){
		Position::state &s =pos.getActualState();
		if(s.checkers){
			stagedGeneratorState=getTTevasion;
		}
		else{
			stagedGeneratorState=getProbCutTT;
		}
		captureThreshold=Position::pieceValue[capturePiece%Position::separationBitmap][0];
		if(ttMove.packed && (!pos.isCaptureMove(ttMove) || !pos.see(ttMove)<captureThreshold)){
			ttMove=0;
		}
	}

	static void initMovegenConstant(void);
	template<Movegen::genType type>	void generateMoves();
	static bool isMoveLegal(const Position&p, Move &m);
	inline unsigned int getGeneratedMoveNumber(void){ return moveListSize;}

	Move  getNextMove(void);


	inline static bitMap attackFrom(Position::bitboardIndex piece,tSquare from,bitMap & occupancy){
		assert(piece<Position::lastBitboard);
		assert(piece!=Position::occupiedSquares);
		assert(piece!=Position::whitePieces);
		assert(piece!=Position::blackPieces);
		assert(piece!=Position::blackPieces);
		assert(piece!=Position::separationBitmap);
		assert(from<squareNumber);
		switch(piece){
		case Position::whiteKing:
		case Position::blackKing:
			return attackFromKing(from);
			break;
		case Position::whiteQueens:
		case Position::blackQueens:
			assert(from<squareNumber);
			return attackFromRook(from,occupancy) | attackFromBishop(from,occupancy);
			break;
		case Position::whiteRooks:
		case Position::blackRooks:
			assert(from<squareNumber);
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
		assert(from <squareNumber);
		assert((((occupancy & MG_RANKMASK[from]) >> RANKSHIFT[from]))<64);
		assert((((occupancy & MG_FILEMASK[from])*MG_FILEMAGIC[from]) >> 57)<64);
		bitMap res = MG_RANK_ATTACK[from][((occupancy & MG_RANKMASK[from]) >> RANKSHIFT[from])];
		res |= MG_FILE_ATTACK[from][((occupancy & MG_FILEMASK[from])*MG_FILEMAGIC[from]) >> 57];
		return res;
	}
	inline static bitMap attackFromBishop(tSquare from,const bitMap & occupancy){
		assert(from <squareNumber);
		assert((((occupancy & MG_DIAGA8H1MASK[from])* MG_DIAGA8H1MAGIC[from])>>57)<64);
		assert((((occupancy & MG_DIAGA1H8MASK[from])*MG_DIAGA1H8MAGIC[from]) >>57)<64);
		bitMap res= MG_DIAGA8H1_ATTACK[from][((occupancy & MG_DIAGA8H1MASK[from])* MG_DIAGA8H1MAGIC[from])>>57];
		res |= MG_DIAGA1H8_ATTACK[from][((occupancy & MG_DIAGA1H8MASK[from])*MG_DIAGA1H8MAGIC[from]) >> 57];
		return res;
	}

	inline static bitMap attackFromKnight(tSquare from){
		assert(from <squareNumber);
		return KNIGHT_MOVE[from];
	}
	inline static bitMap attackFromKing(tSquare from){
		assert(from <squareNumber);
		return KING_MOVE[from];
	}
	inline static bitMap attackFromPawn(tSquare from,unsigned int color ){
		assert(color <=1);
		assert(from <squareNumber);
		return PAWN_ATTACK[color][from];
	}

	inline static bitMap getRookPseudoAttack(tSquare from){
		assert(from<squareNumber);
		return ROOK_PSEUDO_ATTACK[from];
	}

	inline static bitMap getBishopPseudoAttack(tSquare from){
		assert(from<squareNumber);
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
