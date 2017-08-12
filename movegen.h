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
#include "search.h"
#include "history.h"
#include "bitops.h"
#include "magicmoves.h"


class Movegen{
private:

	extMove moveList[MAX_MOVE_PER_POSITION];
	unsigned int moveListSize;
	unsigned int moveListPosition;

	extMove badCaptureList[MAX_BAD_MOVE_PER_POSITION];
	unsigned int badCaptureSize;
	unsigned int badCapturePosition;

	unsigned int killerPos;
	Score captureThreshold;


	const Position &pos;
	const Search &src;
	unsigned int ply;
	Move ttMove;

	Move killerMoves[2];
	Move counterMoves[2];



	enum CastleSide
	{
		kingSideCastle,
		queenSideCastle
	};

	enum genType
	{
		captureMg,			// generate capture moves
		quietMg,			// generate quiet moves
		quietChecksMg,		// generate quiet moves giving check
		allNonEvasionMg,	// generate all moves while not in check
		allEvasionMg,		// generate all moves while in check
		allMg,				// general generate all move
		captureEvasionMg,	// generate capture while in check
		quietEvasionMg		// generate quiet moves while in check


	};

	enum eStagedGeneratorState
	{
		getTT,
		generateCaptureMoves,
		iterateGoodCaptureMoves,
		getKillers,
		getCounters,
		generateQuietMoves,
		iterateQuietMoves,
		iterateBadCaptureMoves,
		finishedNormalStage,

		getTTevasion,
		generateCaptureEvasionMoves,
		iterateCaptureEvasionMoves,
		generateQuietEvasionMoves,
		iterateQuietEvasionMoves,
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
		iterateQuietChecks,
		finishedQuiescentQuietStage,

	}stagedGeneratorState;

	template<Movegen::genType type>	void generateMoves();

	void insertMove(const Move& m)
	{
		assert(moveListSize<MAX_MOVE_PER_POSITION);
		moveList[moveListSize++].m=m;
	}

	inline void scoreCaptureMoves()
	{
		for(unsigned int i = moveListPosition; i < moveListSize; i++)
		{
			moveList[i].score = pos.getMvvLvaScore(moveList[i].m);
		}
	}

	inline void scoreQuietMoves()
	{
		for(unsigned int i = moveListPosition; i < moveListSize; i++)
		{
			moveList[i].score = src.getHistory().getValue(pos.getPieceAt((tSquare)moveList[i].m.bit.from),(tSquare)moveList[i].m.bit.to);
		}
	}

	inline void scoreQuietEvasion()
	{
		for(unsigned int i = moveListPosition; i < moveListSize; i++)
		{
			moveList[i].score = - (pos.getPieceAt((tSquare)moveList[i].m.bit.from));
			if(pos.getPieceAt((tSquare)moveList[i].m.bit.from)== Position::King)
			{
				moveList[i].score = 10;
			}
		}
	}

	inline void resetMoveList()
	{
		moveListPosition = 0;
		moveListSize = 0;
	}


	inline void FindNextBestMove()
	{
		Score bestScore = -SCORE_INFINITE;
		unsigned int bestIndex = moveListPosition;
		for(unsigned int i = moveListPosition; i < moveListSize; i++) // itera sulle mosse rimanenti
		{
			Score res = moveList[i].score;
			if(res > bestScore)
			{
				bestScore = res;
				bestIndex = i;
			}
		}
		if(bestIndex != moveListPosition)
		{
			std::swap(moveList[moveListPosition], moveList[bestIndex]);
		}
	}


public:
	const static Move NOMOVE;
	unsigned int getNumberOfLegalMoves();

	inline unsigned int getGeneratedMoveNumber(void)const { return moveListSize;}

	bool isKillerMove(Move &m) const
	{
		return m == killerMoves[0] || m == killerMoves[1];
	}

	Move getMoveFromMoveList(unsigned int n) const {	return moveList[n].m; }

	Move getNextMove(void);






	Movegen(const Position & p, const Search& s, unsigned int ply, const Move & ttm): pos(p),src(s),ply(ply), ttMove(ttm)
	{
		if(pos.isInCheck())
		{
			stagedGeneratorState = getTTevasion;
		}
		else
		{
			stagedGeneratorState = getTT;
		}
		moveListPosition = 0;
		moveListSize = 0;
		badCaptureSize = 0;
		badCapturePosition = 0;

	}

	Movegen(const Position & p): Movegen(p, defaultSearch, 0, NOMOVE)
	{
	}


	int setupQuiescentSearch(const bool inCheck,const int depth)
	{
		if(inCheck)
		{
			stagedGeneratorState = getTTevasion;
			return (-1*ONE_PLY);
		}
		else
		{
			if(depth >= (0*ONE_PLY))
			{
				stagedGeneratorState = getQsearchTTquiet;
				return -1*ONE_PLY;
			}
			else
			{
				stagedGeneratorState = getQsearchTT;
				if(ttMove.packed && !pos.isCaptureMove(ttMove))
				{
					ttMove = NOMOVE;
				}
				return (-2*ONE_PLY);
			}
		}
	}

	void setupProbCutSearch(Position::bitboardIndex capturePiece)
	{
		/*if(pos.isInCheck())
		{
			stagedGeneratorState = getTTevasion;
		}
		else*/
		{
			stagedGeneratorState = getProbCutTT;
		}

		captureThreshold = Position::pieceValue[capturePiece][0];
		if(ttMove.packed && ((!pos.isCaptureMove(ttMove)) || (pos.see(ttMove) < captureThreshold)))
		{
			ttMove = NOMOVE;
		}
	}







	static void initMovegenConstant(void);

	template<Position::bitboardIndex piece> inline static bitMap attackFrom(const tSquare& from,const bitMap & occupancy=0xffffffffffffffff)
	{
		assert(piece<Position::lastBitboard);
		assert(piece!=Position::occupiedSquares);
		assert(piece!=Position::whitePieces);
		assert(piece!=Position::blackPieces);
		assert(piece!=Position::blackPieces);
		assert(piece!=Position::separationBitmap);
		assert(from<squareNumber);
		switch(piece)
		{
		case Position::whiteKing:
		case Position::blackKing:
			return attackFromKing(from);
			break;
		case Position::whiteQueens:
		case Position::blackQueens:
			assert(from<squareNumber);
			return attackFromQueen(from,occupancy);
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









	inline static const bitMap& getRookPseudoAttack(const tSquare& from)
	{
		assert(from<squareNumber);
		return ROOK_PSEUDO_ATTACK[from];
	}

	inline static const bitMap& getBishopPseudoAttack(const tSquare& from)
	{
		assert(from<squareNumber);
		return BISHOP_PSEUDO_ATTACK[from];
	}
	inline static const bitMap& getCastlePath(const int x, const int y)
	{
		return castlePath[x][y];
	}

private:


	inline static bitMap attackFromRook(const tSquare& from,const bitMap & occupancy)
	{
		assert(from <squareNumber);
		//return Rmagic(from,occupancy);
		return *(magicmoves_r_indices[from]+(((occupancy&magicmoves_r_mask[from])*magicmoves_r_magics[from])>>magicmoves_r_shift[from]));

	}

	inline static bitMap attackFromBishop(const tSquare from,const bitMap & occupancy)
	{
		assert(from <squareNumber);
		return *(magicmoves_b_indices[from]+(((occupancy&magicmoves_b_mask[from])*magicmoves_b_magics[from])>>magicmoves_b_shift[from]));
		//return Bmagic(from,occupancy);

	}
	inline static bitMap attackFromQueen(const tSquare from,const bitMap & occupancy)
	{
		return attackFromRook(from,occupancy) | attackFromBishop(from,occupancy);
	}
	inline static const bitMap& attackFromKnight(const tSquare& from)
	{
		assert(from <squareNumber);
		return KNIGHT_MOVE[from];
	}
	inline static const bitMap& attackFromKing(const tSquare& from)
	{
		assert(from <squareNumber);
		return KING_MOVE[from];
	}
	inline static const bitMap& attackFromPawn(const tSquare& from,const unsigned int& color )
	{
		assert(color <=1);
		assert(from <squareNumber);
		return PAWN_ATTACK[color][from];
	}


	// Move generator magic multiplication numbers for files:
	static bitMap KNIGHT_MOVE[squareNumber];
	static bitMap KING_MOVE[squareNumber];
	static bitMap PAWN_ATTACK[2][squareNumber];
	static bitMap ROOK_PSEUDO_ATTACK[squareNumber];
	static bitMap BISHOP_PSEUDO_ATTACK[squareNumber];
	static bitMap castlePath[2][2];



};


#endif /* MOVEGEN_H_ */
