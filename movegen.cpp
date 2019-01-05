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

#include <list>

#include "bitops.h"
#include "movegen.h"
#include "position.h"
#include "vajolet.h"


bitMap Movegen::_KNIGHT_MOVE[squareNumber];
bitMap Movegen::_KING_MOVE[squareNumber];
bitMap Movegen::_PAWN_ATTACK[2][squareNumber];

bool Movegen::_isValidCoordinate( const int tofile, const int torank )
{
	return (tofile >= 0) & (tofile <= 7) & (torank >= 0) & (torank <= 7);
}

void Movegen::_setBit( bitMap& b, tFile file, tRank rank )
{
	if( _isValidCoordinate( file, rank ) )
	{
		b |= bitSet( getSquare(file,rank) );
	}
}

void Movegen::initMovegenConstant(void){
	
	initmagicmoves();
	
	struct coord{ int x; int y;};
	std::list<coord> pawnsAttack[2] ={{{-1,1},{1,1}},{{-1,-1},{1,-1}}};
	std::list<coord> knightAttack ={{-2,1},{-1,2},{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1}};
	std::list<coord> kingAttack ={{-1,0},{-1,1},{-1,-1},{0,1},{0,-1},{1,0},{1,-1},{1,1}};
	
	for ( tSquare square = A1; square < squareNumber; ++square )
	{
		tFile file = getFileOf(square);
		tRank rank = getRankOf(square);
		
		// pawn attacks
		for( int color = 0; color < 2; ++color)
		{
			_PAWN_ATTACK[color][square] = 0x0;
			for( auto c: pawnsAttack[color] )
			{
				_setBit( _PAWN_ATTACK[color][square], file + c.x, rank + c.y );
			}
		}
		
		// knight moves
		_KNIGHT_MOVE[square] = 0x0;
		for( auto c: knightAttack )
		{
			_setBit( _KNIGHT_MOVE[square], file + c.x, rank + c.y );
		}
		
		// king moves;
		_KING_MOVE[square]= 0x0;
		for( auto c: kingAttack )
		{
			_setBit( _KING_MOVE[square], file + c.x, rank + c.y );
		}
	}
}



template<Movegen::genType type>
void Movegen::generateMoves( MoveList<MAX_MOVE_PER_POSITION>& ml ) const
{

	// initialize constants
	const state &s =_pos.getActualStateConst();
	const bitMap& enemy = _pos.getTheirBitmap(Pieces);
	const bitMap& occupiedSquares = _pos.getOccupationBitmap();
	
	Color color = s.isBlackTurn()? black : white;

	//divide pawns
	const bitMap& seventhRankMask = rankMask( color ? A2:A7);

	bitMap promotionPawns =  _pos.getOurBitmap(Pawns) & seventhRankMask ;
	bitMap nonPromotionPawns =  _pos.getOurBitmap(Pawns)^ promotionPawns;

	const tSquare kingSquare = _pos.getSquareOfOurKing();
	assert(kingSquare<squareNumber);
	
	

	// populate the target squares bitmaps
	bitMap kingTarget;
	bitMap target;
	if constexpr (type==Movegen::allEvasionMg)
	{
		assert(s.getCheckers());
		target = ( s.getCheckers() | getSquaresBetween(kingSquare, firstOne( s.getCheckers() ) ) ) & ~_pos.getOurBitmap(Pieces);
		kingTarget = ~_pos.getOurBitmap(Pieces);
	}
	else if constexpr (type==Movegen::captureEvasionMg)
	{
		assert(s.getCheckers());
		target = ( s.getCheckers() ) & ~_pos.getOurBitmap(Pieces);
		kingTarget = target | _pos.getTheirBitmap(Pieces);
	}
	else if constexpr (type==Movegen::quietEvasionMg)
	{
		assert( s.getCheckers() );
		target = ( getSquaresBetween( kingSquare, firstOne( s.getCheckers() ) ) ) & ~_pos.getOurBitmap(Pieces);
		kingTarget = ~occupiedSquares;
	}
	else if constexpr (type== Movegen::allNonEvasionMg)
	{
		target= ~_pos.getOurBitmap(Pieces);
		kingTarget= target;
	}
	else if constexpr (type== Movegen::captureMg)
	{
		target = _pos.getTheirBitmap(Pieces);
		kingTarget = target;
	}
	else if constexpr (type== Movegen::quietMg)
	{
		target = ~occupiedSquares;
		kingTarget = target;
	}
	else if constexpr (type== Movegen::quietChecksMg)
	{
		target = ~occupiedSquares;
		kingTarget = target;
	}else
	{
		assert(false);
		assert(s.getCheckers());
		target = ( s.getCheckers() | getSquaresBetween( kingSquare, firstOne( s.getCheckers() ) ) ) & ~_pos.getOurBitmap(Pieces);
		kingTarget = ~_pos.getOurBitmap(Pieces);
	}


	

	//------------------------------------------------------
	// king
	//------------------------------------------------------
	bitboardIndex piece = s.getKingOfActivePlayer();
	generateKingMoves<type>( ml, kingSquare, occupiedSquares, kingTarget, enemy );
	
	// if the king is in check from 2 enemy, it can only run away, we should not search any other move
	if((type == Movegen::allEvasionMg || type == Movegen::captureEvasionMg || type == Movegen::quietEvasionMg) && s.isInDoubleCheck() )
	{
		return;
	}
	//------------------------------------------------------
	// queen
	//------------------------------------------------------
	generatePieceMoves<type>( ml, &_attackFromQueen, ++piece, kingSquare, occupiedSquares, target );
	
	//------------------------------------------------------
	// rook
	//------------------------------------------------------
	generatePieceMoves<type>( ml, &_attackFromRook, ++piece, kingSquare, occupiedSquares, target );
	
	//------------------------------------------------------
	// bishop
	//------------------------------------------------------
	generatePieceMoves<type>( ml, &_attackFromBishop, ++piece, kingSquare, occupiedSquares, target );

	//------------------------------------------------------
	// knight
	//------------------------------------------------------
	generatePieceMoves<type>( ml, &_attackFromKnight, ++piece, kingSquare, occupiedSquares, target );

	
	//------------------------------------------------------
	// Pawns
	//------------------------------------------------------
	if constexpr ( type != Movegen::captureMg && type != Movegen::captureEvasionMg )
	{
		//push
		bitMap pawnPushed = generatePawnPushes<type,false>( ml, color, nonPromotionPawns, kingSquare, occupiedSquares, target );
		//double push
		generatePawnDoublePushes<type>( ml, color, pawnPushed, kingSquare, occupiedSquares, target );
	}

	if constexpr (type!= Movegen::quietMg && type!=Movegen::quietChecksMg && type != Movegen::quietEvasionMg)
	{
		//left capture
		generatePawnCaptureLeft<type,false>( ml, color, nonPromotionPawns, kingSquare, target, enemy );

		//right capture
		generatePawnCaptureRight<type,false>( ml, color, nonPromotionPawns, kingSquare, target, enemy );

	}
	
	// PROMOTIONS
	if(type != Movegen::captureMg && type != Movegen::captureEvasionMg)
	{
		//push
		generatePawnPushes<type,true>( ml, color, promotionPawns, kingSquare, occupiedSquares, target );
	}

	if( type!= Movegen::quietMg && type!= Movegen::quietChecksMg && type!= Movegen::quietEvasionMg)
	{
		//left capture
		generatePawnCaptureLeft<type,true>( ml, color, promotionPawns, kingSquare, target, enemy );

		//right capture
		generatePawnCaptureRight<type,true>( ml, color, promotionPawns, kingSquare, target, enemy );

		// ep capture
		generateEpMove( ml, color, nonPromotionPawns, occupiedSquares, kingSquare );
	}

	//king castle
	if(type !=Movegen::allEvasionMg && type!=Movegen::captureEvasionMg && type!=Movegen::quietEvasionMg && type!= Movegen::captureMg)
	{
		if( !s.isInCheck() && s.hasCastleRight( castleOO | castleOOO, color ) )
		{
			generateCastleOO<type>( ml, color, kingSquare, occupiedSquares );
			generateCastleOOO<type>( ml, color, kingSquare, occupiedSquares );
		}
	}
}
template void Movegen::generateMoves<Movegen::captureMg>( MoveList<MAX_MOVE_PER_POSITION>& ml ) const;
template void Movegen::generateMoves<Movegen::quietMg>( MoveList<MAX_MOVE_PER_POSITION>& ml ) const;
template void Movegen::generateMoves<Movegen::quietChecksMg>( MoveList<MAX_MOVE_PER_POSITION>& ml ) const;

template<Movegen::genType type>
inline void Movegen::insertStandardMove( MoveList<MAX_MOVE_PER_POSITION>& ml, const Move& m ) const
{
	if( type !=Movegen::quietChecksMg || _pos.moveGivesCheck( m ) )
	{
		ml.insert( m );
	}
}

void Movegen::insertPromotionMoves( MoveList<MAX_MOVE_PER_POSITION>& ml, Move& m ) const
{
	for(Move::epromotion prom=Move::promQueen; prom<= Move::promKnight; prom=(Move::epromotion)( prom+1) )
	{
		m.setPromotion( prom );
		ml.insert(m);
	}
}

template<Movegen::genType type>
inline void Movegen::generateKingMoves( MoveList<MAX_MOVE_PER_POSITION>& ml, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap kingTarget, const bitMap enemy )const
{
	Move m(Move::NOMOVE);
	
	m.setFrom( kingSquare );

	bitMap moves = _attackFromKing(kingSquare,occupiedSquares) & kingTarget;

	while(moves)
	{
		tSquare to = iterateBit(moves);
		if( !(_pos.getAttackersTo(to, occupiedSquares & ~_pos.getOurBitmap(King)) & enemy) )
		{
			m.setTo( to );
			insertStandardMove<type>( ml, m );
		}
	}
}

template<Movegen::genType type>
inline void Movegen::generatePieceMoves( MoveList<MAX_MOVE_PER_POSITION>& ml, bitMap (*attack)(const tSquare,const bitMap&), const bitboardIndex piece, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap target )const
{
	
	Move m(Move::NOMOVE);
	bitMap bFrom = _pos.getBitmap(piece);
	while(bFrom)
	{
		tSquare from = iterateBit(bFrom);
		assert(from < squareNumber);
		m.setFrom( from );

		bitMap moves = attack(from,occupiedSquares) & target;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			if( !_pos.getActualStateConst().isPinned( from ) || squaresAligned(from, to, kingSquare))
			{
				m.setTo( to );
				insertStandardMove<type>( ml, m );
			}
		}
	}
}

template<Movegen::genType type, bool promotion>
inline bitMap Movegen::generatePawnPushes( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap target )const
{
	bitMap pawnPushed;
	//push
	bitMap moves = ( color? (pawns>>8):(pawns<<8)) & ~occupiedSquares;
	pawnPushed = moves;
	moves &= target;
	
	Move m(Move::NOMOVE);
	if( promotion )
	{
		m.setFlag( Move::fpromotion );
	}

	while(moves)
	{
		tSquare to = iterateBit(moves);
		tSquare from = to - pawnPush( color );
		
		if( !_pos.getActualStateConst().isPinned( from ) || squaresAligned(from,to,kingSquare))
		{
			m.setTo( to );
			m.setFrom( from );
			if( promotion )
			{
				insertPromotionMoves( ml, m );
			}
			else
			{
				insertStandardMove<type>( ml, m );
			}
		}
	}
	return pawnPushed;
}

template<Movegen::genType type>
inline void Movegen::generatePawnDoublePushes( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap occupiedSquares, const bitMap target )const
{
	//double push
	const bitMap& thirdRankMask = rankMask( color ? A6:A3);
	bitMap moves = ( color? ((pawns & thirdRankMask)>>8):((pawns & thirdRankMask)<<8)) & ~occupiedSquares & target;
	Move m(Move::NOMOVE);
	while(moves)
	{
		tSquare to = iterateBit(moves);
		tSquare from = to - 2*pawnPush( color );

		if( !_pos.getActualStateConst().isPinned( from ) || squaresAligned(from ,to ,kingSquare))
		{
			m.setTo( to );
			m.setFrom( from );
			insertStandardMove<type>( ml, m );
		}
	}
}
template<Movegen::genType type, bool promotion>
inline void Movegen::generatePawnCaptureLeft( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap target, const bitMap enemy )const
{
	//left capture
	int delta = color? -9: 7;
	bitMap moves = ( color? (pawns&~fileMask(A1))>>9: (pawns&~fileMask(A1))<<7) & enemy & target;
	generatePawnCapture<type, promotion>( ml, delta, moves, kingSquare );
}

template<Movegen::genType type, bool promotion>
inline void Movegen::generatePawnCaptureRight( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const tSquare kingSquare, const bitMap target, const bitMap enemy )const
{
	//right capture
	int delta= color? -7: 9;
	bitMap moves = ( color? (pawns&~fileMask(H1))>>7: (pawns&~fileMask(H1))<<9) & enemy & target;
	generatePawnCapture<type, promotion>( ml, delta, moves, kingSquare );
}

template<Movegen::genType type, bool promotion>
inline void Movegen::generatePawnCapture( MoveList<MAX_MOVE_PER_POSITION>& ml, int delta, bitMap moves, const tSquare kingSquare)const
{
	Move m(Move::NOMOVE);
	if( promotion )
	{
		m.setFlag( Move::fpromotion );
	}
	while(moves)
	{
		tSquare to = iterateBit(moves);
		tSquare from = (tSquare)(to - delta);

		if( !_pos.getActualStateConst().isPinned( from ) || squaresAligned(from,to,kingSquare))
		{
			m.setTo( to );
			m.setFrom( from );
			
			if( promotion )
			{
				insertPromotionMoves( ml, m );
			}
			else
			{
				ml.insert(m);
			}
		}
	}
}

inline void Movegen::generateEpMove(MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const bitMap& pawns, const bitMap occupiedSquares, const tSquare kingSquare) const
{
	if( _pos.getActualStateConst().hasEpSquare() )
	{
		auto epSquare = _pos.getActualStateConst().getEpSquare();
		Move m(Move::NOMOVE);
		m.setFlag( Move::fenpassant );
		bitMap epAttacker = pawns & _attackFromPawn( epSquare, 1 - color );

		while(epAttacker)
		{
			tSquare from = iterateBit(epAttacker);

			bitMap captureSquare= fileMask( epSquare ) & rankMask(from);
			bitMap occ = occupiedSquares^bitSet(from)^bitSet( epSquare )^captureSquare;

			if(	!((_attackFromRook(kingSquare, occ) & (_pos.getTheirBitmap(Queens) | _pos.getTheirBitmap(Rooks))) |
				(_attackFromBishop(kingSquare, occ) & (_pos.getTheirBitmap(Queens) | _pos.getTheirBitmap(Bishops))))
			)
			{
				m.setTo( epSquare );
				m.setFrom( from );
				ml.insert(m);
			}
		}

	}
}

template<>
void Movegen::generateMoves<Movegen::allMg>( MoveList<MAX_MOVE_PER_POSITION>& ml ) const
{

	if(_pos.isInCheck())
	{
		generateMoves<Movegen::captureEvasionMg>( ml);
		generateMoves<Movegen::quietEvasionMg>( ml );
	}
	else
	{
		generateMoves<Movegen::genType::captureMg>( ml );
		generateMoves<Movegen::genType::quietMg>( ml );
	}

}

template<Movegen::genType type>
inline void Movegen::generateCastleOO( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const tSquare kingSquare, const bitMap occupiedSquares )const
{
	eCastle cr = state::calcCastleRight( castleOO, color );
	if( _pos.getActualStateConst().hasCastleRight( cr ) && _pos.isCastlePathFree( cr ) )
	{

		bool castleDenied = false;
		for( tSquare x = (tSquare)1; x<3; x++)
		{
			assert(kingSquare+x<squareNumber);
			if(_pos.getAttackersTo(kingSquare+x,occupiedSquares) & _pos.getTheirBitmap(Pieces))
			{
				castleDenied = true;
				break;
			}
		}
		if(!castleDenied)
		{
			Move m(Move::NOMOVE);
			m.setFlag( Move::fcastle );
			m.setFrom( kingSquare );
			m.setTo( (tSquare)(kingSquare + 2) );
			if(type !=Movegen::quietChecksMg || _pos.moveGivesCheck(m))
			{
				ml.insert(m);
			}
		}
	}
}

template<Movegen::genType type>
inline void Movegen::generateCastleOOO( MoveList<MAX_MOVE_PER_POSITION>& ml, const Color color, const tSquare kingSquare, const bitMap occupiedSquares )const
{
	eCastle cr = state::calcCastleRight( castleOOO, color );
	if( _pos.getActualStateConst().hasCastleRight( cr ) && _pos.isCastlePathFree( cr ) )
	{
		bool castleDenied = false;
		for( tSquare x = (tSquare)1 ;x<3 ;x++)
		{
			assert(kingSquare-x<squareNumber);
			if(_pos.getAttackersTo(kingSquare-x, occupiedSquares) & _pos.getTheirBitmap(Pieces))
			{
				castleDenied = true;
				break;
			}
		}
		if(!castleDenied)
		{
			Move m(Move::NOMOVE);
			m.setFlag( Move::fcastle );
			m.setFrom( kingSquare );
			m.setTo( (tSquare)(kingSquare - 2) );
			if(type != Movegen::quietChecksMg || _pos.moveGivesCheck(m))
			{
				ml.insert(m);
			}
		}
	}
}