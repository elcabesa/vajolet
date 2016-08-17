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
#include "vajolet.h"
#include "move.h"
#include "movegen.h"
#include "data.h"
#include "bitops.h"
#include "history.h"
#include "magicmoves.h"


const Move Movegen::NOMOVE(0);



bitMap Movegen::KNIGHT_MOVE[squareNumber];
bitMap Movegen::KING_MOVE[squareNumber];
bitMap Movegen::PAWN_ATTACK[2][squareNumber];

bitMap Movegen::ROOK_PSEUDO_ATTACK[squareNumber];
bitMap Movegen::BISHOP_PSEUDO_ATTACK[squareNumber];

bitMap Movegen::castlePath[2][2];


void Movegen::initMovegenConstant(void){

	castlePath[0][kingSideCastle]=bitSet(F1)|bitSet(G1);
	castlePath[0][queenSideCastle]=bitSet(D1)|bitSet(C1)|bitSet(B1);
	castlePath[1][kingSideCastle]=bitSet(F8)|bitSet(G8);
	castlePath[1][queenSideCastle]=bitSet(D8)|bitSet(C8)|bitSet(B8);

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
	const Position::state &s =pos.getActualState();
	const bitMap& enemy = pos.getTheirBitmap(Position::Pieces);
	const bitMap& occupiedSquares = pos.getOccupationBitmap();

	//divide pawns
	const bitMap& thirdRankMask = RANKMASK[ s.nextMove ? A6:A3];
	const bitMap& seventhRankMask = RANKMASK[ s.nextMove ? A2:A7];

	bitMap promotionPawns =  pos.getOurBitmap(Position::Pawns) & seventhRankMask ;
	bitMap nonPromotionPawns =  pos.getOurBitmap(Position::Pawns)^ promotionPawns;

	const tSquare kingSquare = pos.getSquareOfThePiece((Position::bitboardIndex)(Position::whiteKing+s.nextMove),0);
	assert(kingSquare<squareNumber);

	// populate the target squares bitmaps
	bitMap kingTarget;
	bitMap target;
	if(type==Movegen::allEvasionMg)
	{
		assert(s.checkers);
		target = ( s.checkers | SQUARES_BETWEEN[kingSquare][firstOne(s.checkers)]) &~ pos.getOurBitmap(Position::Pieces);
		kingTarget = ~pos.getOurBitmap(Position::Pieces);
	}
	else if(type==Movegen::captureEvasionMg)
	{
		assert(s.checkers);
		target = ( s.checkers ) &~ pos.getOurBitmap(Position::Pieces);
		kingTarget = target | pos.getTheirBitmap(Position::Pieces);
		//displayBitmap(target);
	}
	else if(type==Movegen::quietEvasionMg)
	{
		assert(s.checkers);
		target = ( SQUARES_BETWEEN[kingSquare][firstOne(s.checkers)]) &~ pos.getOurBitmap(Position::Pieces);
		kingTarget = ~pos.getOccupationBitmap();
		//displayBitmap(target);
		//displayBitmap(kingTarget);
	}
	else if(type== Movegen::allNonEvasionMg)
	{
		target= ~pos.getOurBitmap(Position::Pieces);
		kingTarget= target;
	}
	else if(type== Movegen::captureMg)
	{
		target = pos.getTheirBitmap(Position::Pieces);
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
	}


	bitMap moves;
	Move m(NOMOVE);
	//------------------------------------------------------
	// king
	//------------------------------------------------------
	Position::bitboardIndex piece = (Position::bitboardIndex)( s.nextMove + Position::whiteKing );
	assert(pos.isKing(piece));
	assert(piece<Position::lastBitboard);

	{
		m.bit.from = kingSquare;

		moves = attackFromKing(kingSquare) & kingTarget;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			m.bit.to = to;

			if( !(pos.getAttackersTo(to, pos.getOccupationBitmap() & ~pos.getOurBitmap(Position::King)) & enemy) )
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}
	}
	// if the king is in check from 2 enemy, it can only run away, we sohld not search any other move
	if((type == Movegen::allEvasionMg || type == Movegen::captureEvasionMg || type == Movegen::quietEvasionMg) && moreThanOneBit(s.checkers))
	{
		return;
	}


	//------------------------------------------------------
	// queen
	//------------------------------------------------------
	piece = (Position::bitboardIndex)( piece+1 );

	for(unsigned int i = 0; i < pos.getPieceCount(piece); i++)
	{

		assert(i<Position::maxNumberOfPieces);
		tSquare from = pos.getSquareOfThePiece(piece,i);
		assert(from < squareNumber);
		m.bit.from = from;

		moves = attackFromQueen(from,occupiedSquares) & target;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			m.bit.to = to;

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from, to, kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}
	}

	//------------------------------------------------------
	// rook
	//------------------------------------------------------
	piece= (Position::bitboardIndex)(piece+1);

	for(unsigned int i=0; i<pos.getPieceCount(piece); i++)
	{
		assert(i<Position::maxNumberOfPieces);
		tSquare from = pos.getSquareOfThePiece(piece,i);
		assert(from < squareNumber);
		m.bit.from = from;

		moves = attackFromRook(from,occupiedSquares) & target;

		while(moves)
		{
			tSquare to = iterateBit(moves);
			m.bit.to = to;

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from, to, kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}
	}

	//------------------------------------------------------
	// bishop
	//------------------------------------------------------
	piece = (Position::bitboardIndex)(piece+1);

	for(unsigned int i=0; i<pos.getPieceCount(piece); i++)
	{
		assert(i<Position::maxNumberOfPieces);
		tSquare from = pos.getSquareOfThePiece(piece,i);
		assert(from < squareNumber);
		m.bit.from = from;

		moves = attackFromBishop(from,occupiedSquares) & target;

		while (moves)
		{
			tSquare to = iterateBit(moves);
			m.bit.to = to;

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from,to,kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}
	}




	//------------------------------------------------------
	// knight
	//------------------------------------------------------
	piece = (Position::bitboardIndex)(piece+1);

	for(unsigned int i=0; i<pos.getPieceCount(piece); i++)
	{
		assert(i<Position::maxNumberOfPieces);
		tSquare from = pos.getSquareOfThePiece(piece,i);
		assert(from<squareNumber);
		m.bit.from = from;

		if(!(s.pinnedPieces & bitSet(from)))
		{
			moves = attackFromKnight(from) & target;
			while (moves)
			{
				m.bit.to=iterateBit(moves);

				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}
	}

	//------------------------------------------------------
	// Pawns
	//------------------------------------------------------
	piece = (Position::bitboardIndex)(piece+1);
	if(type != Movegen::captureMg && type != Movegen::captureEvasionMg)
	{
		bitMap pawnPushed;
		//push
		moves = (s.nextMove? (nonPromotionPawns>>8):(nonPromotionPawns<<8)) & ~occupiedSquares;
		pawnPushed = moves;
		moves &= target;
		//displayBitmap(moves);

		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = to - pawnPush(s.nextMove);

			m.bit.to= to;
			m.bit.from = from;

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from,to,kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}

		//double push
		moves = (s.nextMove? ((pawnPushed & thirdRankMask)>>8):((pawnPushed & thirdRankMask)<<8)) & ~occupiedSquares & target;

		//displayBitmap(moves);
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = to - 2*pawnPush(s.nextMove);

			m.bit.to = to;
			m.bit.from = from;
			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from ,to ,kingSquare))
			{
				if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
				{
					insertMove(m);
				}
			}
		}
	}

	int delta;

	if(type!= Movegen::quietMg && type!=Movegen::quietChecksMg && type != Movegen::quietEvasionMg)
	{
		//left capture
		delta = s.nextMove?-9:7;

		moves = (s.nextMove?(nonPromotionPawns&(~FILEMASK[A1]))>>9:(nonPromotionPawns&(~FILEMASK[A1]))<<7) & enemy & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to - delta);

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from,to,kingSquare))
			{
				m.bit.to = to;
				m.bit.from = from;
				insertMove(m);
			}
		}

		//right capture
		delta=s.nextMove?-7:9;

		moves = (s.nextMove?(nonPromotionPawns&(~FILEMASK[H1]))>>7:(nonPromotionPawns&(~FILEMASK[H1]))<<9) & enemy & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to -delta);


			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from,to,kingSquare))
			{
				m.bit.to = to;
				m.bit.from = from;
				insertMove(m);
			}
		}
	}

	// PROMOTIONS
	m.bit.flags = Move::fpromotion;
	if(type != Movegen::captureMg && type != Movegen::captureEvasionMg)
	{
		moves = (s.nextMove? (promotionPawns>>8):(promotionPawns<<8))& ~occupiedSquares & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = to - pawnPush(s.nextMove);

			m.bit.to = to;
			m.bit.from = from;

			if(!(s.pinnedPieces & bitSet(from)) ||	squaresAligned(from,to,kingSquare))
			{
				for(Move::epromotion prom=Move::promQueen; prom<= Move::promKnight; prom=(Move::epromotion)(prom+1))
				{
					m.bit.promotion = prom;
					insertMove(m);
				}
			}
		}
	}

	int color = s.nextMove?1:0;

	if( type!= Movegen::quietMg && type!= Movegen::quietChecksMg && type!= Movegen::quietEvasionMg)
	{
		//left capture
		delta = s.nextMove?-9:7;
		moves = (s.nextMove?(promotionPawns&(~FILEMASK[A1]))>>9:(promotionPawns&(~FILEMASK[A1]))<<7) & enemy & target;
		while(moves)
		{
			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to -delta);

			m.bit.to=to;
			m.bit.from=from;

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from,to,kingSquare))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1))
				{
					m.bit.promotion = prom;
					insertMove(m);
				}
			}
		}

		//right capture
		delta=s.nextMove?-7:9;
		moves = (s.nextMove?(promotionPawns&(~FILEMASK[H1]))>>7:(promotionPawns&(~FILEMASK[H1]))<<9) & enemy & target;
		while(moves)
		{

			tSquare to = iterateBit(moves);
			tSquare from = (tSquare)(to -delta);

			m.bit.to=to;
			m.bit.from=from;

			if(!(s.pinnedPieces & bitSet(from)) || squaresAligned(from,to,kingSquare))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1))
				{
					m.bit.promotion = prom;
					insertMove(m);
				}
			}
		}

		m.bit.promotion = 0;
		m.bit.flags = Move::fnone;

		// ep capture

		if(s.epSquare != squareNone)
		{
			m.bit.flags = Move::fenpassant;
			bitMap epAttacker = nonPromotionPawns & attackFromPawn(s.epSquare,1-color);

			while(epAttacker)
			{
				tSquare from = iterateBit(epAttacker);

				bitMap captureSquare= FILEMASK[s.epSquare] & RANKMASK[from];
				bitMap occ = occupiedSquares^bitSet(from)^bitSet(s.epSquare)^captureSquare;

				if(	!((attackFromRook(kingSquare, occ) & (pos.getTheirBitmap(Position::Queens) | pos.getTheirBitmap(Position::Rooks))) |
						(Movegen::attackFromBishop(kingSquare, occ) & (pos.getTheirBitmap(Position::Queens) | pos.getTheirBitmap(Position::Bishops))))
				)
				{
					m.bit.to = s.epSquare;
					m.bit.from = from;
					insertMove(m);
				}
			}

		}
	}




	//king castle
	if(type !=Movegen::allEvasionMg && type!=Movegen::captureEvasionMg && type!=Movegen::quietEvasionMg && type!= Movegen::captureMg)
	{

		if(s.castleRights & ((Position::wCastleOO |Position::wCastleOOO)<<(2*color)))
		{

			if((s.castleRights &((Position::wCastleOO)<<(2*color))) &&!s.checkers &&!(castlePath[color][kingSideCastle] & pos.getOccupationBitmap()))
			{

				bool castleDenied = false;
				for( tSquare x = (tSquare)1; x<3; x++)
				{
					assert(kingSquare+x<squareNumber);
					if(pos.getAttackersTo(kingSquare+x,pos.getOccupationBitmap()) & pos.getTheirBitmap(Position::Pieces))
					{
						castleDenied = true;
						break;
					}
				}
				if(!castleDenied)
				{
					m.bit.flags = Move::fcastle;
					m.bit.from = kingSquare;
					m.bit.to = kingSquare + 2;
					if(type !=Movegen::quietChecksMg || pos.moveGivesCheck(m))
					{
						insertMove(m);
					}
				}


			}
			if((s.castleRights &((Position::wCastleOOO)<<(2*color))) && !s.checkers && !(castlePath[color][queenSideCastle] & pos.getOccupationBitmap()))
			{
				bool castleDenied = false;
				for( tSquare x = (tSquare)1 ;x<3 ;x++)
				{
					assert(kingSquare-x<squareNumber);
					if(pos.getAttackersTo(kingSquare-x, pos.getOccupationBitmap()) & pos.getTheirBitmap(Position::Pieces))
					{
						castleDenied = true;
						break;
					}
				}
				if(!castleDenied)
				{
					m.bit.flags = Move::fcastle;
					m.bit.from = kingSquare;
					m.bit.to = kingSquare - 2;
					if(type != Movegen::quietChecksMg || pos.moveGivesCheck(m))
					{
						insertMove(m);
					}
				}
			}
		}
	}
	assert(moveListSize<=MAX_MOVE_PER_POSITION);



}
template void Movegen::generateMoves<Movegen::captureMg>();
template void Movegen::generateMoves<Movegen::quietMg>();
template void Movegen::generateMoves<Movegen::quietChecksMg>();



template<>
void Movegen::generateMoves<Movegen::allMg>()
{

	if(pos.isInCheck())
	{
		//generateMoves<Movegen::allEvasionMg>();
		generateMoves<Movegen::captureEvasionMg>();
		generateMoves<Movegen::quietEvasionMg>();
	}
	else
	{
		generateMoves<Movegen::genType::captureMg>();
		generateMoves<Movegen::genType::quietMg>();
	}

}

unsigned int Movegen::getNumberOfLegalMoves()
{
	generateMoves<Movegen::allMg>();
	return getGeneratedMoveNumber();
}


Move Movegen::getNextMove()
{

	while(true)
	{
		switch(stagedGeneratorState)
		{
		case generateCaptureMoves:
		case generateQuiescentMoves:
		case generateQuiescentCaptures:
		case generateProbCutCaptures:

			generateMoves<Movegen::genType::captureMg>();

			scoreCaptureMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case generateQuietMoves:

			resetMoveList();

			generateMoves<Movegen::genType::quietMg>();

			scoreQuietMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case generateCaptureEvasionMoves:
		{

			//generateMoves<Movegen::allEvasionMg>();
			generateMoves<Movegen::captureEvasionMg>();
			//generateMoves<Movegen::quietEvasionMg>();

			// non usate dalla generazione delle mosse, ma usate dalla ricerca!!
			killerMoves[0] = (src.getKillers(ply,0));
			killerMoves[1] = (src.getKillers(ply,1));

			scoreCaptureMoves();

			/*Move previousMove = pos.getActualState().currentMove;
			if(previousMove.packed)
			{
				counterMoves[0] = src.getCounterMove().getMove(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, 0);
				counterMoves[1] = src.getCounterMove().getMove(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, 1);
			}
			else
			{
				counterMoves[0] = NOMOVE;
				counterMoves[1] = NOMOVE;
			}*/


			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
		}
			break;
		case generateQuietEvasionMoves:
		{

			//generateMoves<Movegen::allEvasionMg>();
			//generateMoves<Movegen::captureEvasionMg>();
			generateMoves<Movegen::quietEvasionMg>();
			scoreQuietMoves();

			// non usate dalla generazione delle mosse, ma usate dalla ricerca!!
			//killerMoves[0] = (src.getKillers(ply,0));
			//killerMoves[1] = (src.getKillers(ply,1));

			/*Move previousMove = pos.getActualState().currentMove;
			if(previousMove.packed)
			{
				counterMoves[0] = src.getCounterMove().getMove(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, 0);
				counterMoves[1] = src.getCounterMove().getMove(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, 1);
			}
			else
			{
				counterMoves[0] = NOMOVE;
				counterMoves[1] = NOMOVE;
			}*/


			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
		}
			break;

		case generateQuietCheks:

			resetMoveList();

			generateMoves<Movegen::quietChecksMg>();

			scoreQuietMoves();

			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case iterateQuietMoves:
			if( moveListPosition < moveListSize )
			{
				FindNextBestMove();

				if(moveList[moveListPosition].m != ttMove && !isKillerMove(moveList[moveListPosition].m) && moveList[moveListPosition].m != counterMoves[0] &&  moveList[moveListPosition].m != counterMoves[1])
				{
					//if(moveList[moveListPosition].score > 0 || this->depth >= 3* ONE_PLY )
					//{
						return moveList[moveListPosition++].m;
					//}
					//else
					//{
					//	moveListPosition++;
					//	// TODO qui posso pasa allo stage successivo
					//}
				}
				else
				{
					moveListPosition++;
				}
			}
			else
			{
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case iterateCaptureEvasionMoves:
			if(moveListPosition < moveListSize)
			{
				FindNextBestMove();
				if(moveList[moveListPosition].m != ttMove)
				{
					return moveList[moveListPosition++].m;
				}
				moveListPosition++;
			}
			else
			{
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case iterateQuietEvasionMoves:
		if(moveListPosition < moveListSize)
		{
			FindNextBestMove();
			if(moveList[moveListPosition].m != ttMove)
			{
				return moveList[moveListPosition++].m;
			}
			moveListPosition++;
		}
		else
		{
			stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
		}
		break;
		case iterateQuietChecks:
			if(moveListPosition < moveListSize)
			{
				FindNextBestMove();

				if(moveList[moveListPosition].m != ttMove)
				{
					return moveList[moveListPosition++].m;
				}
				else
				{
					moveListPosition++;
				}
			}
			else
			{
				stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);
			}


			break;
		case iterateGoodCaptureMoves:
			if(moveListPosition < moveListSize)
			{
				FindNextBestMove();

				if(moveList[moveListPosition].m != ttMove)
				{
					if((pos.seeSign(moveList[moveListPosition].m)>=0) || (pos.moveGivesSafeDoubleCheck(moveList[moveListPosition].m)))
					{
						return moveList[moveListPosition++].m;
					}
					else
					{
						assert(badCaptureSize<MAX_BAD_MOVE_PER_POSITION);
						badCaptureList[badCaptureSize++].m = moveList[moveListPosition++].m;
					}
				}
				else
				{
					moveListPosition++;
				}

			}
			else
			{
				killerMoves[0] = src.getKillers(ply, 0);
				killerMoves[1] = src.getKillers(ply, 1);

				Move previousMove = pos.getActualState().currentMove;
				if(previousMove.packed)
				{
					counterMoves[0] = src.getCounterMove().getMove(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, 0);
					counterMoves[1] = src.getCounterMove().getMove(pos.getPieceAt((tSquare)previousMove.bit.to), (tSquare)previousMove.bit.to, 1);
				}
				else
				{
					counterMoves[0] = NOMOVE;
					counterMoves[1] = NOMOVE;
				}

				killerPos = 0;
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case iterateProbCutCaptures:
			if(moveListPosition < moveListSize)
			{
				FindNextBestMove();

				if(moveList[moveListPosition].m != ttMove)
				{
					if(pos.see(moveList[moveListPosition].m) >= captureThreshold)
					{
						return moveList[moveListPosition++].m;
					}
					moveListPosition++;
				}
				else
				{
					moveListPosition++;
				}

			}
			else
			{
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;
		case iterateBadCaptureMoves:
			if(badCapturePosition < badCaptureSize)
			{
				return badCaptureList[badCapturePosition++].m;
			}
			else{
				stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);
			}
			break;


		case iterateQuiescentMoves:
		case iterateQuiescentCaptures:
			if(moveListPosition < moveListSize)
			{
				FindNextBestMove();

				if(moveList[moveListPosition].m != ttMove)
				{
					return moveList[moveListPosition++].m;
				}
				else
				{
					moveListPosition++;
				}
			}
			else
			{
				stagedGeneratorState = (eStagedGeneratorState)(stagedGeneratorState+1);
			}
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

				if((t != ttMove) && (t != killerMoves[0]) && (t != killerMoves[1]) && !pos.isCaptureMove(t) && pos.isMoveLegal(t))
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
			if(ttMove.packed && pos.isMoveLegal(ttMove))
			{
				return ttMove;
			}
			break;
		default:
			return NOMOVE;
			break;
		}
	}

	return NOMOVE;


}
