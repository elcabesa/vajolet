
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

#include <utility>
#include <algorithm>
#include <array>

#include "bitops.h"
#include "eCastle.h"
#include "magicmoves.h"
#include "MoveList.h"
#include "position.h"
#include "search.h"
#include "vajolet.h"

extern SearchData defaultSearchData;

class Movegen
{
private:
	static const int MAX_MOVE_PER_POSITION = 250;
	static const int MAX_BAD_MOVE_PER_POSITION = 32;

	MoveList<MAX_MOVE_PER_POSITION> moveList;
	MoveList<MAX_BAD_MOVE_PER_POSITION> badCaptureList;

	unsigned int killerPos;
	Score captureThreshold;


	const Position &pos;
	const SearchData &_sd;
	unsigned int ply;
	Move ttMove;

	Move killerMoves[2];
	Move counterMoves[2];




public:
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
private:
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

public: 
	template<Movegen::genType type>	void generateMoves( MoveList<MAX_MOVE_PER_POSITION>& ml)const;
private:
	void scoreCaptureMoves();
	void scoreQuietMoves();
	void scoreQuietEvasion();
	unsigned int getGeneratedMoveNumber( MoveList<MAX_MOVE_PER_POSITION>& ml )const;


public:
	unsigned int getNumberOfLegalMoves();

	bool isKillerMove(Move &m) const
	{
		return m == killerMoves[0] || m == killerMoves[1];
	}
	Move getNextMove(void);

	Movegen(const Position & p, const SearchData& sd = defaultSearchData, unsigned int ply = 0, const Move & ttm = Move::NOMOVE): pos(p),_sd(sd),ply(ply), ttMove(ttm)
	{
		if(pos.isInCheck())
		{
			stagedGeneratorState = getTTevasion;
		}
		else
		{
			stagedGeneratorState = getTT;
		}
		//moveList.reset();
		//badCaptureList.reset();

	}



	int setupQuiescentSearch(const bool inCheck,const int depth)
	{
		if(inCheck)
		{
			stagedGeneratorState = getTTevasion;
			return -1;
		}
		else
		{
			if( depth >= 0 )
			{
				stagedGeneratorState = getQsearchTTquiet;
				return -1;
			}
			else
			{
				
				stagedGeneratorState = getQsearchTT;
				if( ttMove && /*pos.isMoveLegal(ttMove)&&  */!pos.isCaptureMove(ttMove))
				{
					ttMove = Move::NOMOVE;
				}
				return -2;
			}
		}
	}

	void setupProbCutSearch(bitboardIndex capturePiece)
	{
		//if(pos.isInCheck())
		//{
		//	stagedGeneratorState = getTTevasion;
		//}
		//else
		//{
			stagedGeneratorState = getProbCutTT;
		//}

		captureThreshold = Position::pieceValue[capturePiece][0];
		if(pos.isMoveLegal(ttMove) && ((!pos.isCaptureMove(ttMove)) || (pos.see(ttMove) < captureThreshold)))
		{
			ttMove = Move::NOMOVE;
		}
	}







	static void initMovegenConstant(void);

	template<bitboardIndex piece> inline static bitMap attackFrom(const tSquare& from,const bitMap & occupancy=0xffffffffffffffff)
	{
		assert( isValidPiece( piece ));
		assert(from<squareNumber);
		switch(piece)
		{
		case whiteKing:
		case blackKing:
			return attackFromKing(from);
			break;
		case whiteQueens:
		case blackQueens:
			assert(from<squareNumber);
			return attackFromQueen(from,occupancy);
			break;
		case whiteRooks:
		case blackRooks:
			assert(from<squareNumber);
			return attackFromRook(from,occupancy);
			break;
		case whiteBishops:
		case blackBishops:
			return attackFromBishop(from,occupancy);
			break;
		case whiteKnights:
		case blackKnights:
			return attackFromKnight(from);
			break;
		case whitePawns:
			return attackFromPawn(from,0);
			break;
		case blackPawns:
			return attackFromPawn(from,1);
			break;
		default:
			return 0;

		}

	}

	inline static bitMap getRookPseudoAttack(const tSquare& from)
	{
		assert(from<squareNumber);
		return attackFromRook(from,0);
	}

	inline static bitMap getBishopPseudoAttack(const tSquare& from)
	{
		assert(from<squareNumber);
		return attackFromBishop(from,0);
	}
	bool isCastlePathFree( const eCastle c ) const;

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
	static std::array<bitMap,9> castlePath;



};


#endif /* MOVEGEN_H_ */
