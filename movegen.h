
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

#include "magicmoves.h"
#include "MoveList.h"

class Position;

class Movegen
{
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
	
	Movegen(const Position & p): _pos(p){}
	
	template<Movegen::genType type>	void generateMoves( MoveList<MAX_MOVE_PER_POSITION>& ml)const;
	
	static void initMovegenConstant(void);
	
	template<bitboardIndex piece> inline static bitMap attackFrom(const tSquare& from,const bitMap& occupancy=0xffffffffffffffff)
	{
		assert( isValidPiece( piece ));
		assert(from<squareNumber);
		switch( piece )
		{
		case whiteKing:
		case blackKing:
			return _attackFromKing(from, occupancy);
			break;
		case whiteQueens:
		case blackQueens:
			assert(from<squareNumber);
			return _attackFromQueen(from,occupancy);
			break;
		case whiteRooks:
		case blackRooks:
			assert(from<squareNumber);
			return _attackFromRook(from,occupancy);
			break;
		case whiteBishops:
		case blackBishops:
			return _attackFromBishop(from,occupancy);
			break;
		case whiteKnights:
		case blackKnights:
			return _attackFromKnight(from,occupancy);
			break;
		case whitePawns:
			return _attackFromPawn(from,0);
			break;
		case blackPawns:
			return _attackFromPawn(from,1);
			break;
		default:
			return 0;

		}

	}

	inline static bitMap getRookPseudoAttack(const tSquare& from)
	{
		assert(from<squareNumber);
		return _attackFromRook(from,0);
	}

	inline static bitMap getBishopPseudoAttack(const tSquare& from)
	{
		assert(from<squareNumber);
		return _attackFromBishop(from,0);
	}
	
private:

	const Position &_pos;
	
	// Move generator magic multiplication numbers for files:
	static bitMap _KNIGHT_MOVE[squareNumber];
	static bitMap _KING_MOVE[squareNumber];
	static bitMap _PAWN_ATTACK[2][squareNumber];

	inline static bitMap _attackFromRook(const tSquare from, const bitMap& occupancy)
	{
		assert(from <squareNumber);
		return *(magicmoves_r_indices[from]+(((occupancy&magicmoves_r_mask[from])*magicmoves_r_magics[from])>>magicmoves_r_shift[from]));
	}

	inline static bitMap _attackFromBishop(const tSquare from, const bitMap& occupancy)
	{
		assert(from <squareNumber);
		return *(magicmoves_b_indices[from]+(((occupancy&magicmoves_b_mask[from])*magicmoves_b_magics[from])>>magicmoves_b_shift[from]));
	}
	
	inline static bitMap _attackFromQueen(const tSquare from, const bitMap& occupancy)
	{
		return _attackFromRook(from,occupancy) | _attackFromBishop(from,occupancy);
	}
	
	inline static bitMap _attackFromKnight(const tSquare from, const bitMap&)
	{
		assert(from <squareNumber);
		return _KNIGHT_MOVE[from];
	}
	
	inline static bitMap _attackFromKing(const tSquare from, const bitMap&)
	{
		assert(from <squareNumber);
		return _KING_MOVE[from];
	}
	
	inline static bitMap _attackFromPawn(const tSquare from ,const unsigned int& color )
	{
		assert(color <=1);
		assert(from <squareNumber);
		return _PAWN_ATTACK[color][from];
	}
	
	static bool _isValidCoordinate( const int tofile, const int torank );
	static void _setBit( bitMap& b, tFile file, tRank rank );
	
	template<Movegen::genType type>	void insertStandardMove( MoveList<MAX_MOVE_PER_POSITION>& ml, const Move& m ) const;
	void insertPromotionMoves( MoveList<MAX_MOVE_PER_POSITION>& ml, Move& m ) const;
	
	template<Movegen::genType type>	void generateKingMoves( MoveList<MAX_MOVE_PER_POSITION>& ml, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap kingTarget, const bitMap enemy )const;
	
	template<Movegen::genType type>	void generatePieceMoves( MoveList<MAX_MOVE_PER_POSITION>& ml, bitMap (*attack)(const tSquare,const bitMap&),const bitboardIndex piece, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap target)const;
	
	template<Movegen::genType type, bool promotion> bitMap generatePawnPushes( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap target )const;
	
	template<Movegen::genType type> void generatePawnDoublePushes( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap target )const;
	
	template<Movegen::genType type, bool promotion> void generatePawnCaptureLeft( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap target, const bitMap enemy )const;
	
	template<Movegen::genType type, bool promotion> void generatePawnCaptureRight( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap target, const bitMap enemy )const;
	
	template<Movegen::genType type, bool promotion> void generatePawnCapture( MoveList<MAX_MOVE_PER_POSITION>& ml, int delta, bitMap moves, const tSquare kingSquare)const;
	
	inline void generateEpMove(MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const bitMap occupiedSquares, const tSquare kingSquare) const;
	
	template<Movegen::genType type>void generateCastleOO( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const tSquare kingSquare, const bitMap occupiedSquares )const;
	template<Movegen::genType type>void generateCastleOOO( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const tSquare kingSquare, const bitMap occupiedSquares )const;
};


#endif /* MOVEGEN_H_ */
