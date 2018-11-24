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


#include <functional>

#include "bitops.h"
#include "data.h"
#include "movegen.h"
#include "vajolet.h"

// todo create empty ogject?
SearchData defaultSearchData;


bitMap Movegen::KNIGHT_MOVE[squareNumber];
bitMap Movegen::KING_MOVE[squareNumber];
bitMap Movegen::PAWN_ATTACK[2][squareNumber];

bitMap Movegen::ROOK_PSEUDO_ATTACK[squareNumber];
bitMap Movegen::BISHOP_PSEUDO_ATTACK[squareNumber];

std::array<bitMap,9> Movegen::castlePath;



void Movegen::initMovegenConstant(void){

	for( auto& x : castlePath )
	{
		x = 0;
	}
	castlePath.at( wCastleOO  ) = bitSet(F1) | bitSet(G1);
	castlePath.at( wCastleOOO ) = bitSet(D1) | bitSet(C1) | bitSet(B1);
	castlePath.at( bCastleOO  ) = bitSet(F8) | bitSet(G8);
	castlePath.at( bCastleOOO ) = bitSet(D8) | bitSet(C8) | bitSet(B8);

	initmagicmoves();


	for (int square = 0; square < squareNumber; square++)
	{
		KNIGHT_MOVE[square] = 0x0;
		KING_MOVE[square]= 0x0;
		PAWN_ATTACK[0][square] = 0x0;
		PAWN_ATTACK[1][square] = 0x0;

	}

	// pawn attacks
	for (int square = 0; square < squareNumber; square++)
	{
		int file = FILES[square];
		int rank = RANKS[square];
		int tofile = file - 1;
		int torank = rank + 1;
		if ((tofile >= 0) & (tofile <= 7) & (torank >= 0) & (torank <= 7)){
			PAWN_ATTACK[0][square] |= bitSet(BOARDINDEX[tofile][torank]);
		}
		tofile = file + 1;
		torank = rank + 1;
		if ((tofile >= 0) & (tofile <= 7) & (torank >= 0) & (torank <= 7))
			PAWN_ATTACK[0][square] |= bitSet(BOARDINDEX[tofile][torank]);
		tofile = file - 1;
		torank = rank - 1;
		if ((tofile >= 0) & (tofile <= 7) & (torank >= 0) & (torank <= 7))
			PAWN_ATTACK[1][square] |= bitSet(BOARDINDEX[tofile][torank]);
		tofile = file + 1;
		torank = rank - 1;
		if ((tofile >= 0) & (tofile <= 7) & (torank >= 0) & (torank <= 7))
			PAWN_ATTACK[1][square] |= bitSet(BOARDINDEX[tofile][torank]);
	}


	// KNIGHT attacks;
	for (int square = 0; square < squareNumber; square++)
	{
		int file = FILES[square];
		int rank = RANKS[square];
		int toFile = file - 2;
		int toRank = rank + 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file - 1; toRank = rank + 2;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 1; toRank = rank + 2;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 2; toRank = rank + 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 2; toRank = rank - 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 1; toRank = rank - 2;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file - 1; toRank = rank - 2;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file - 2; toRank = rank - 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KNIGHT_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
	}

	// KING attacks;
	for (int square = 0; square < squareNumber; square++)
	{
		int file = FILES[square];
		int rank = RANKS[square];
		int toFile = file - 1;
		int toRank = rank;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file - 1; toRank = rank + 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file; toRank = rank + 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 1; toRank = rank + 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 1; toRank = rank;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file + 1; toRank = rank - 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file; toRank = rank - 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
		toFile = file - 1; toRank = rank - 1;
		if ((toFile >= 0) & (toFile <= 7) & (toRank >= 0) & (toRank <= 7))
			KING_MOVE[square] |= bitSet(BOARDINDEX[toFile][toRank]);
	}

	for (unsigned int square = 0; square < squareNumber; square++){
		bitMap x=0;
		ROOK_PSEUDO_ATTACK[square] = attackFromRook((tSquare)square,x);
		BISHOP_PSEUDO_ATTACK[square] = attackFromBishop((tSquare)square,x);
	}
}



template<Movegen::genType type>
void Movegen::generateMoves()
{

	// initialize constants
	const state &s =pos.getActualStateConst();
	const bitMap& enemy = pos.getTheirBitmap(Pieces);
	const bitMap& occupiedSquares = pos.getOccupationBitmap();

	//divide pawns
	const bitMap& thirdRankMask = RANKMASK[ s.isBlackTurn() ? A6:A3];
	const bitMap& seventhRankMask = RANKMASK[ s.isBlackTurn() ? A2:A7];

	bitMap promotionPawns =  pos.getOurBitmap(Pawns) & seventhRankMask ;
	bitMap nonPromotionPawns =  pos.getOurBitmap(Pawns)^ promotionPawns;

	const tSquare kingSquare = pos.getSquareOfThePiece( s.getKingOfActivePlayer() );
	assert(kingSquare<squareNumber);

	// populate the target squares bitmaps
	bitMap kingTarget;
	bitMap target;
	if(type==Movegen::allEvasionMg)
	{
		assert(s.getCheckers());
		target = ( s.getCheckers() | SQUARES_BETWEEN[kingSquare][ firstOne( s.getCheckers() ) ]) & ~pos.getOurBitmap(Pieces);
		kingTarget = ~pos.getOurBitmap(Pieces);
	}
	else if(type==Movegen::captureEvasionMg)
	{
		assert(s.getCheckers());
		target = ( s.getCheckers() ) & ~pos.getOurBitmap(Pieces);
		kingTarget = target | pos.getTheirBitmap(Pieces);
		//displayBitmap(target);
	}
	else if(type==Movegen::quietEvasionMg)
	{
		assert( s.getCheckers() );
		target = ( SQUARES_BETWEEN[kingSquare] [firstOne( s.getCheckers() ) ]) & ~pos.getOurBitmap(Pieces);
		kingTarget = ~pos.getOccupationBitmap();
		//displayBitmap(target);
		//displayBitmap(kingTarget);
	}
	else if(type== Movegen::allNonEvasionMg)
	{
		target= ~pos.getOurBitmap(Pieces);
		kingTarget= target;
	}
	else if(type== Movegen::captureMg)
	{
		target = pos.getTheirBitmap(Pieces);
		kingTarget = target;
	}
	else if(type== Movegen::quietMg)
	{
		target = ~pos.getOccupationBitmap();
		kingTarget = target;
	}
	else if(type== Movegen::quietChecksMg)
	{
		target = ~pos.getOccupationBitmap();
		kingTarget = target;
	}else
	{
		assert(false);
		assert(s.getCheckers());
		target = ( s.getCheckers() | SQUARES_BETWEEN[kingSquare][ firstOne( s.getCheckers() ) ]) & ~pos.getOurBitmap(Pieces);
		kingTarget = ~pos.getOurBitmap(Pieces);
	}


	bitMap moves;
	Move m(Move::NOMOVE);
	//------------------------------------------------------
	// king
	//------------------------------------------------------
	bitboardIndex piece = s.getKingOfActivePlayer();
	assert( isKing(piece) );
	assert( isValidPiece(piece) );

	{
		m.setFrom( kingSquare );

		moves = attackFromKing(kingSquare) & kingTarget;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			m.setTo( to );

			if( !(pos.getAttackersTo(to, pos.getOccupationBitmap() & ~pos.getOurBitmap(King)) & enemy) )
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}
	}
	// if the king is in check from 2 enemy, it can only run away, we should not search any other move
	if((type == Movegen::allEvasionMg || type == Movegen::captureEvasionMg || type == Movegen::quietEvasionMg) && s.isInDoubleCheck() )
	{
		return;
	}


	//------------------------------------------------------
	// queen
	//------------------------------------------------------
	piece = (bitboardIndex)( piece+1 );
	bitMap bFrom = pos.getBitmap(piece);
	while(bFrom)
	{
		tSquare from = iterateBit(bFrom);
		assert(from < squareNumber);
		m.setFrom( from );

		moves = attackFromQueen(from,occupiedSquares) & target;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			m.setTo( to );

			if( !s.isPinned( from ) || squaresAligned(from, to, kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}
	}

	//------------------------------------------------------
	// rook
	//------------------------------------------------------
	piece= (bitboardIndex)(piece+1);
	bFrom = pos.getBitmap(piece);
	while(bFrom)
	{
		tSquare from = iterateBit(bFrom);
		assert(from < squareNumber);
		m.setFrom( from );

		moves = attackFromRook(from,occupiedSquares) & target;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			m.setTo( to );

			if( !s.isPinned( from ) || squaresAligned(from, to, kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}
	}

	//------------------------------------------------------
	// bishop
	//------------------------------------------------------
	piece = (bitboardIndex)(piece+1);
	bFrom = pos.getBitmap(piece);
	while(bFrom)
	{
		tSquare from = iterateBit(bFrom);
		assert(from < squareNumber);
		m.setFrom( from );

		moves = attackFromBishop(from,occupiedSquares) & target;

		while (moves)
		{
			tSquare to = iterateBit(moves);
			m.setTo( to );

			if( !s.isPinned( from ) || squaresAligned(from,to,kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}
	}




	//------------------------------------------------------
	// knight
	//------------------------------------------------------
	piece = (bitboardIndex)(piece+1);
	bFrom = pos.getBitmap(piece);
	while(bFrom)
	{
		tSquare from = iterateBit(bFrom);
		assert(from<squareNumber);
		m.setFrom( from );

		if( !s.isPinned( from ) )
		{
			moves = attackFromKnight(from) & target;
			while (moves)
			{
				m.setTo( iterateBit(moves) );

				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}
	}

	//------------------------------------------------------
	// Pawns
	//------------------------------------------------------
	piece = (bitboardIndex)(piece+1);
	if(type != Movegen::captureMg && type != Movegen::captureEvasionMg)
	{
		bitMap pawnPushed;
		//push
		moves = (s.isBlackTurn()? (nonPromotionPawns>>8):(nonPromotionPawns<<8)) & ~occupiedSquares;
		pawnPushed = moves;
		moves &= target;
		//displayBitmap(moves);

		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = to - pawnPush(s.isBlackTurn());

			m.setTo( to );
			m.setFrom( from );

			if( !s.isPinned( from ) || squaresAligned(from,to,kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}

		//double push
		moves = (s.isBlackTurn()? ((pawnPushed & thirdRankMask)>>8):((pawnPushed & thirdRankMask)<<8)) & ~occupiedSquares & target;

		//displayBitmap(moves);
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = to - 2*pawnPush(s.isBlackTurn());

			m.setTo( to );
			m.setFrom( from );
			if( !s.isPinned( from ) || squaresAligned(from ,to ,kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					moveList.insert(m);
				}
			}
		}
	}

	int delta;

	if(type!= Movegen::quietMg && type!=Movegen::quietChecksMg && type != Movegen::quietEvasionMg)
	{
		//left capture
		delta = s.isBlackTurn()?-9:7;

		moves = (s.isBlackTurn()?(nonPromotionPawns&(~FILEMASK[A1]))>>9:(nonPromotionPawns&(~FILEMASK[A1]))<<7) & enemy & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to - delta);

			if( !s.isPinned( from ) || squaresAligned(from,to,kingSquare))
			{
				m.setTo( to );
				m.setFrom( from );
				moveList.insert(m);
			}
		}

		//right capture
		delta=s.isBlackTurn()?-7:9;

		moves = (s.isBlackTurn()?(nonPromotionPawns&(~FILEMASK[H1]))>>7:(nonPromotionPawns&(~FILEMASK[H1]))<<9) & enemy & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to -delta);


			if( !s.isPinned( from ) || squaresAligned(from,to,kingSquare))
			{
				m.setTo( to );
				m.setFrom( from );
				moveList.insert(m);
			}
		}
	}

	// PROMOTIONS
	m.setFlag( Move::fpromotion );
	if(type != Movegen::captureMg && type != Movegen::captureEvasionMg)
	{
		moves = (s.isBlackTurn()? (promotionPawns>>8):(promotionPawns<<8))& ~occupiedSquares & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = to - pawnPush(s.isBlackTurn());

			m.setTo( to );
			m.setFrom( from );

			if( !s.isPinned( from ) ||	squaresAligned(from,to,kingSquare))
			{
				for(Move::epromotion prom=Move::promQueen; prom<= Move::promKnight; prom=(Move::epromotion)(prom+1))
				{
					m.setPromotion( prom );
					moveList.insert(m);
				}
			}
		}
	}

	Color color = s.isBlackTurn()? black : white;

	if( type!= Movegen::quietMg && type!= Movegen::quietChecksMg && type!= Movegen::quietEvasionMg)
	{
		//left capture
		delta = s.isBlackTurn()?-9:7;
		moves = (s.isBlackTurn()?(promotionPawns&(~FILEMASK[A1]))>>9:(promotionPawns&(~FILEMASK[A1]))<<7) & enemy & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to -delta);

			m.setTo( to );
			m.setFrom( from );

			if( !s.isPinned( from ) || squaresAligned(from,to,kingSquare))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1))
				{
					m.setPromotion( prom );
					moveList.insert(m);
				}
			}
		}

		//right capture
		delta=s.isBlackTurn()?-7:9;
		moves = (s.isBlackTurn()?(promotionPawns&(~FILEMASK[H1]))>>7:(promotionPawns&(~FILEMASK[H1]))<<9) & enemy & target;
		while(moves)
		{

			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to -delta);

			m.setTo( to );
			m.setFrom( from );

			if( !s.isPinned( from ) || squaresAligned(from,to,kingSquare))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1))
				{
					m.setPromotion( prom );
					moveList.insert(m);
				}
			}
		}

		m.setPromotion( Move::promQueen );
		m.setFlag( Move::fnone );

		// ep capture

		if( s.hasEpSquare() )
		{
			auto epSquare = s.getEpSquare();
			m.setFlag( Move::fenpassant );
			bitMap epAttacker = nonPromotionPawns & attackFromPawn( epSquare, 1-color );

			while(epAttacker)
			{
				tSquare from = iterateBit(epAttacker);

				bitMap captureSquare= FILEMASK[ epSquare ] & RANKMASK[from];
				bitMap occ = occupiedSquares^bitSet(from)^bitSet( epSquare )^captureSquare;

				if(	!((attackFromRook(kingSquare, occ) & (pos.getTheirBitmap(Queens) | pos.getTheirBitmap(Rooks))) |
						(Movegen::attackFromBishop(kingSquare, occ) & (pos.getTheirBitmap(Queens) | pos.getTheirBitmap(Bishops))))
				)
				{
					m.setTo( epSquare );
					m.setFrom( from );
					moveList.insert(m);
				}
			}

		}
	}

	//king castle
	if(type !=Movegen::allEvasionMg && type!=Movegen::captureEvasionMg && type!=Movegen::quietEvasionMg && type!= Movegen::captureMg)
	{
		m.setPromotion( Move::promQueen );
		if( !s.isInCheck() && s.hasCastleRight( castleOO | castleOOO, color ) )
		{
			eCastle cr = state::calcCastleRight( castleOO, color );
			if( s.hasCastleRight( cr ) && isCastlePathFree( cr ) )
			{

				bool castleDenied = false;
				for( tSquare x = (tSquare)1; x<3; x++)
				{
					assert(kingSquare+x<squareNumber);
					if(pos.getAttackersTo(kingSquare+x,pos.getOccupationBitmap()) & pos.getTheirBitmap(Pieces))
					{
						castleDenied = true;
						break;
					}
				}
				if(!castleDenied)
				{
					m.setFlag( Move::fcastle );
					m.setFrom( kingSquare );
					m.setTo( (tSquare)(kingSquare + 2) );
					if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
					{
						moveList.insert(m);
					}
				}


			}
			cr = state::calcCastleRight( castleOOO, color );
			if( s.hasCastleRight( cr ) && isCastlePathFree( cr ) )
			{
				bool castleDenied = false;
				for( tSquare x = (tSquare)1 ;x<3 ;x++)
				{
					assert(kingSquare-x<squareNumber);
					if(pos.getAttackersTo(kingSquare-x, pos.getOccupationBitmap()) & pos.getTheirBitmap(Pieces))
					{
						castleDenied = true;
						break;
					}
				}
				if(!castleDenied)
				{
					m.setFlag( Move::fcastle );
					m.setFrom( kingSquare );
					m.setTo( (tSquare)(kingSquare - 2) );
					if(type != Movegen::quietChecksMg || pos.moveGivesCheck(m))
					{
						moveList.insert(m);
					}
				}
			}
		}
	}
}
template void Movegen::generateMoves<Movegen::captureMg>();
template void Movegen::generateMoves<Movegen::quietMg>();
template void Movegen::generateMoves<Movegen::quietChecksMg>();



template<>
void Movegen::generateMoves<Movegen::allMg>()
{

	if(pos.isInCheck())
	{
		generateMoves<Movegen::captureEvasionMg>();
		generateMoves<Movegen::quietEvasionMg>();
	}
	else
	{
		generateMoves<Movegen::genType::captureMg>();
		generateMoves<Movegen::genType::quietMg>();
	}

}

inline unsigned int  Movegen::getGeneratedMoveNumber(void) const
{
	return moveList.size();
}

unsigned int Movegen::getNumberOfLegalMoves()
{
	generateMoves<Movegen::allMg>();
	return getGeneratedMoveNumber();
}


Move Movegen::getNextMove()
{
	Move mm;

	while(true)
	{
		switch(stagedGeneratorState)
		{
		case generateCaptureMoves:
		case generateQuiescentMoves:
		case generateQuiescentCaptures:
		case generateProbCutCaptures:

			generateMoves<Movegen::genType::captureMg>();
			moveList.ignoreMove(ttMove);

			scoreCaptureMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case generateQuietMoves:

			moveList.reset();

			generateMoves<Movegen::genType::quietMg>();
			moveList.ignoreMove(ttMove);
			moveList.ignoreMove(killerMoves[0]);
			moveList.ignoreMove(killerMoves[1]);
			moveList.ignoreMove(counterMoves[0]);
			moveList.ignoreMove(counterMoves[1]);

			scoreQuietMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case generateCaptureEvasionMoves:

			generateMoves<Movegen::captureEvasionMg>();
			moveList.ignoreMove(ttMove);

			// non usate dalla generazione delle mosse, ma usate dalla ricerca!!
			killerMoves[0] = _sd.getKillers(ply,0);
			killerMoves[1] = _sd.getKillers(ply,1);

			scoreCaptureMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case generateQuietEvasionMoves:

			generateMoves<Movegen::quietEvasionMg>();
			moveList.ignoreMove(ttMove);

			scoreQuietEvasion();
			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case generateQuietCheks:

			moveList.reset();
			generateMoves<Movegen::quietChecksMg>();
			moveList.ignoreMove(ttMove);

			scoreQuietMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case iterateQuietMoves:
		case iterateQuiescentCaptures:
		case iterateCaptureEvasionMoves:
		case iterateQuiescentMoves:
		case iterateQuietChecks:
		case iterateQuietEvasionMoves:

			if( ( mm = moveList.findNextBestMove() ) )
			{
				return mm;
			}
			else
			{
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case iterateGoodCaptureMoves:

			if( ( mm = moveList.findNextBestMove() ) )
			{
				if((pos.seeSign(mm)>=0) || (pos.moveGivesSafeDoubleCheck(mm)))
				{
					return mm;
				}
				else
				{
					badCaptureList.insert( mm );
				}

			}
			else
			{
				killerMoves[0] = _sd.getKillers(ply, 0);
				killerMoves[1] = _sd.getKillers(ply, 1);

				if( const Move& previousMove = pos.getActualStateConst().getCurrentMove() )
				{
					counterMoves[0] = _sd.getCounterMove().getMove(pos.getPieceAt(previousMove.getTo()), previousMove.getTo(), 0);
					counterMoves[1] = _sd.getCounterMove().getMove(pos.getPieceAt(previousMove.getTo()), previousMove.getTo(), 1);
				}
				else
				{
					counterMoves[0] = Move::NOMOVE;
					counterMoves[1] = Move::NOMOVE;
				}

				killerPos = 0;
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case iterateProbCutCaptures:
			if( ( mm = moveList.findNextBestMove() ) )
			{
				if(pos.see(mm) >= captureThreshold)
				{
					return mm;
				}
			}
			else
			{
				return Move::NOMOVE;
			}
			break;
		case iterateBadCaptureMoves:
			return badCaptureList.getNextMove();
			break;
		case getKillers:
			if(killerPos < 2)
			{
				Move& t = killerMoves[killerPos++];

				if((t != ttMove) && !pos.isCaptureMove(t) && pos.isMoveLegal(t))
				{
					return t;
				}
			}
			else
			{
				killerPos = 0;
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case getCounters:
			if(killerPos < 2)
			{
				Move& t = counterMoves[killerPos++];

				if((t != ttMove) && !isKillerMove(t) && !pos.isCaptureMove(t) && pos.isMoveLegal(t))
				{
					return t;
				}
			}
			else
			{
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case getTT:
		case getTTevasion:
		case getQsearchTT:
		case getQsearchTTquiet:
		case getProbCutTT:
			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			if(pos.isMoveLegal(ttMove))
			{
				return ttMove;
			}
			break;
		default:
			return Move::NOMOVE;
			break;
		}
	}

	return Move::NOMOVE;


}


inline void Movegen::scoreCaptureMoves()
{
	for( auto& m : moveList )
	{
		Score s = pos.getMvvLvaScore( m )
				+ _sd.getCaptureHistory().getValue( pos.getPieceAt( m.getFrom()) , m , pos.getPieceAt( m.getTo() ) ) * 50; // history of capture
		m.setScore( s );
	}

}

inline void Movegen::scoreQuietMoves()
{
	for( auto& m : moveList )
	{
		m.setScore( _sd.getHistory().getValue( Color(pos.isBlackTurn()), m ) );
	}
}

inline void Movegen::scoreQuietEvasion()
{
	for( auto& m : moveList )
	{
		Score s = ( pos.getPieceAt( m.getFrom() ) );
		if( pos.getPieceAt( m.getFrom()) % separationBitmap == King )
		{
			s = 20;
		}
		s *= 500000;

		s += _sd.getHistory().getValue( Color(pos.isBlackTurn()), m );

		m.setScore( s );
	}
}

bool Movegen::isCastlePathFree( const eCastle c ) const
{
	assert( c < 9);
	return !( castlePath[c] & pos.getOccupationBitmap() );
}


