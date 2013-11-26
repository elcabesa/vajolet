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
#include "movegen.h"
#include "data.h"
#include "bitops.h"


Move Movegen::NOMOVE;
bitMap Movegen::MG_RANKMASK[squareNumber];
bitMap Movegen::MG_FILEMASK[squareNumber];
bitMap Movegen::MG_DIAGA8H1MASK[squareNumber];
bitMap Movegen::MG_DIAGA1H8MASK[squareNumber];
bitMap Movegen::MG_RANK_ATTACK[squareNumber][64];
bitMap Movegen::MG_FILE_ATTACK[squareNumber][64];
bitMap Movegen::MG_DIAGA8H1_ATTACK[squareNumber][64];
bitMap Movegen::MG_DIAGA1H8_ATTACK[squareNumber][64];
bitMap Movegen::MG_FILEMAGIC[64];
bitMap Movegen::MG_DIAGA8H1MAGIC[64];
bitMap Movegen::MG_DIAGA1H8MAGIC[64];

bitMap Movegen::KNIGHT_MOVE[squareNumber];
bitMap Movegen::KING_MOVE[squareNumber];
bitMap Movegen::PAWN_ATTACK[2][squareNumber];

bitMap Movegen::ROOK_PSEUDO_ATTACK[squareNumber];
bitMap Movegen::BISHOP_PSEUDO_ATTACK[squareNumber];

bitMap Movegen::castlePath[2][2];
/*
Move Movegen::moveListPool[1024][MAX_MOVE_PER_POSITION];
unsigned int Movegen::moveListAllocated=0;
*/


const int Movegen::RANKSHIFT[squareNumber] = {
	1,  1,  1,  1,  1,  1,  1,  1,
	9,  9,  9,  9,  9,  9,  9,  9,
	17, 17, 17, 17, 17, 17, 17, 17,
	25, 25, 25, 25, 25, 25, 25, 25,
	33, 33, 33, 33, 33, 33, 33, 33,
	41, 41, 41, 41, 41, 41, 41, 41,
	49, 49, 49, 49, 49, 49, 49, 49,
	57, 57, 57, 57, 57, 57, 57, 57
};

const bitMap Movegen::FILEMAGICS[8] = {
	0x8040201008040200,
	0x4020100804020100,
	0x2010080402010080,
	0x1008040201008040,
	0x0804020100804020,
	0x0402010080402010,
	0x0201008040201008,
	0x0100804020100804
	};

const bitMap Movegen::DIAGA8H1MAGICS[15] = {
	0x0,
	0x0,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0101010101010100,
	0x0080808080808080,
	0x0040404040404040,
	0x0020202020202020,
	0x0010101010101010,
	0x0008080808080808,
	0x0,
	0x0
};

const bitMap Movegen::DIAGA1H8MAGICS[15] = {
       0x0,
       0x0,
       0x0101010101010100,
       0x0101010101010100,
       0x0101010101010100,
       0x0101010101010100,
       0x0101010101010100,
       0x0101010101010100,
       0x8080808080808000,
       0x4040404040400000,
       0x2020202020000000,
       0x1010101000000000,
       0x0808080000000000,
       0x0,
       0x0
};

void Movegen::initMovegenConstant(void){

	castlePath[0][KING_SIDE_CASTLE]=bitSet(F1)|bitSet(G1);
	castlePath[0][QUEEN_SIDE_CASTLE]=bitSet(D1)|bitSet(C1)|bitSet(B1);
	castlePath[1][KING_SIDE_CASTLE]=bitSet(F8)|bitSet(G8);
	castlePath[1][QUEEN_SIDE_CASTLE]=bitSet(D8)|bitSet(C8)|bitSet(B8);


	unsigned char GEN_SLIDING_ATTACKS[8][64];
	// loop over rank, file or diagonal squares:
	for (tSquare square =square0; square <= 7; square++)
	{
		// loop of occupancy states
		// state6Bit represents the 64 possible occupancy states of a rank,
		// except the 2 end-bits, because they don't matter for calculating attacks
		for (unsigned char state6Bit = 0; state6Bit < 64; state6Bit++)
		{
			unsigned char state8Bit = state6Bit << 1; // create an 8-bit occupancy state
			unsigned char attack8Bit = 0;
			if (square < 7)
			{
				attack8Bit |= bitSet(square + (tSquare)1);
			}
			tSquare slide = square + (tSquare)2;
			while (slide <= (tSquare)7) // slide in '+' direction
			{
				if ((~state8Bit) & (bitSet(slide - (tSquare)1)))
				{
					attack8Bit |= bitSet(slide);
				}
				else break;
				slide++;
			}
			if (square > 0)
			{
				attack8Bit |= bitSet(square - (tSquare)1);
			}
			slide = square - (tSquare)2;
			while (slide >= 0) // slide in '-' direction
			{
				if ((~state8Bit) & (bitSet(slide + (tSquare)1)))
				{
					attack8Bit |= bitSet(slide);
				}
				else break;
				slide--;
			}
			GEN_SLIDING_ATTACKS[square][state6Bit] = attack8Bit;
		}
	}

	for (int square = 0; square < squareNumber; square++)
	{
		KNIGHT_MOVE[square] = 0x0;
		KING_MOVE[square]= 0x0;

		MG_RANKMASK[square] = 0x0;
		MG_FILEMASK[square] = 0x0;
		MG_DIAGA8H1MASK[square] = 0x0;
		MG_DIAGA1H8MASK[square] = 0x0;
		MG_DIAGA8H1MAGIC[square] = 0x0;
		MG_DIAGA1H8MAGIC[square] = 0x0;

		PAWN_ATTACK[0][square] = 0x0;
		PAWN_ATTACK[1][square] = 0x0;



		for (int state = 0; state < 64; state++)
		{
			MG_RANK_ATTACK[square][state] = 0x0;
			MG_FILE_ATTACK[square][state] = 0x0;
			MG_DIAGA8H1_ATTACK[square][state] = 0x0;
			MG_DIAGA1H8_ATTACK[square][state] = 0x0;

		}
	}

	// pawn attacks
	for (int square = 0; square < 64; square++)
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

	for (int file = 0; file < 8; file++)
	{
		for (int rank = 0; rank < 8; rank++)
		{
			//===========================================================================
			//initialize 6-bit rank mask, used by movegenerator
			//===========================================================================

			MG_RANKMASK[BOARDINDEX[file][rank]]  = bitSet(BOARDINDEX[1][rank]) | bitSet(BOARDINDEX[2][rank]) | bitSet(BOARDINDEX[3][rank]) ;
			MG_RANKMASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[4][rank]) | bitSet(BOARDINDEX[5][rank]) | bitSet(BOARDINDEX[6][rank]) ;

			//===========================================================================
			//initialize 6-bit file mask, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			MG_FILEMASK[BOARDINDEX[file][rank]]  = bitSet(BOARDINDEX[file][1]) | bitSet(BOARDINDEX[file][2]) | bitSet(BOARDINDEX[file][3]) ;
			MG_FILEMASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[file][4]) | bitSet(BOARDINDEX[file][5]) | bitSet(BOARDINDEX[file][6]) ;

			//===========================================================================
			//Initialize file magic multiplication numbers, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			MG_FILEMAGIC[BOARDINDEX[file][rank]] = FILEMAGICS[file];

			//===========================================================================
			//Initialize 6-bit diagonal mask, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			int diaga8h1 = file + rank; // from 0 to 14, longest diagonal = 7
			MG_DIAGA8H1MAGIC[BOARDINDEX[file][rank]] = DIAGA8H1MAGICS[diaga8h1];

			//===========================================================================
			//Initialize 6-bit diagonal mask, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			if (diaga8h1 < 8)  // lower half, diagonals 0 to 7
			{
				for (int square = 1 ; square < diaga8h1 ; square ++)
				{
					MG_DIAGA8H1MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[square][diaga8h1-square]);
				}
			}
			else  // upper half, diagonals 8 to 14
			{
				for (int square = 1 ; square < 14 - diaga8h1 ; square ++)
				{
					MG_DIAGA8H1MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[diaga8h1+square-7][7-square]);
				}
			}

			//===========================================================================
			//Initialize diagonal magic multiplication numbers, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			int diaga1h8 = file - rank; // from -7 to +7, longest diagonal = 0
			MG_DIAGA1H8MAGIC[BOARDINDEX[file][rank]] = DIAGA1H8MAGICS[diaga1h8+7];

			//===========================================================================
			//Initialize 6-bit diagonal mask, used in the movegenerator (see movegen.ccp)
			//===========================================================================
			if (diaga1h8 > -1)  // lower half, diagonals 0 to 7
			{
				for (int square = 1 ; square < 7 - diaga1h8 ; square ++)
				{
					MG_DIAGA1H8MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[diaga1h8 + square][square]);
				}
			}
			else
			{
				for (int square = 1 ; square < 7 + diaga1h8 ; square ++)
				{
					MG_DIAGA1H8MASK[BOARDINDEX[file][rank]] |= bitSet(BOARDINDEX[square][square - diaga1h8]);
				}
			}


		}
	}

	//  RANK attacks (ROOKS and QUEENS):
	//  use           unsigned char GEN_SLIDING_ATTACKS[8 squares] [64 states]
	//  to initialize BitMap        RANK_ATTACKS       [64 squares][64 states]
	//
	for (int square = 0; square < 64; square++)
	{
		for (int state6Bit = 0; state6Bit < 64; state6Bit++)
		{
			MG_RANK_ATTACK[square][state6Bit] = 0;
			MG_RANK_ATTACK[square][state6Bit] |=
				bitMap(GEN_SLIDING_ATTACKS[FILES[square]][state6Bit]) << (RANKSHIFT[square]-1);

		}
	}

	//  FILE attacks (ROOKS and QUEENS):
	//  use           unsigned char GEN_SLIDING_ATTACKS[8 squares] [64 states]
	//  to initialize BitMap        FILE_ATTACKS       [64 squares][64 states]
	//
	//  Occupancy transformation is as follows:
	//
	//   occupancy state bits of the file:               occupancy state bits in GEN_SLIDING_ATTACKS:
	//
	//        . . . . . . . . MSB                           LSB         MSB
	//        . . . . . A . .                    =>         A B C D E F . .
	//        . . . . . B . .
	//        . . . . . C . .
	//        . . . . . D . .
	//        . . . . . E . .
	//        . . . . . F . .
	//    LSB . . . . . . . .
	//
	//  The reverse transformation is as follows:
	//
	//   attack bits in GEN_SLIDING_ATTACKS:             attack bits in the file:
	//
	//        LSB         MSB                               . . . . . m . . MSB
	//        m n o p q r s t                    =>         . . . . . n . .
	//                                                      . . . . . o . .
	//                                                      . . . . . p . .
	//                                                      . . . . . q . .
	//                                                      . . . . . r . .
	//                                                      . . . . . s . .
	//                                                 LSB  . . . . . t . .
	//
	for (tSquare square = square0; square < squareNumber; square++)
	{
		for (int state6Bit = 0; state6Bit < 64; state6Bit++)
		{
			MG_FILE_ATTACK[square][state6Bit] = 0x0;

			// check to see if attackbit'-th  bit is set in GEN_SLIDING_ATTACKS, for this combination of square/occupancy state
			for (tSquare attackbit = square0; attackbit < 8; attackbit++) // from LSB to MSB
			{
				//  conversion from 64 board squares to the 8 corresponding positions in the GEN_SLIDING_ATTACKS array: "8-RANKS[square]"
				if (GEN_SLIDING_ATTACKS[7-RANKS[square]][state6Bit] &bitSet(attackbit))
				{
					// the bit is set, so we need to update FILE_ATTACKS accordingly:
					// conversion of square/attackbit to the corresponding 64 board FILE: FILES[square]
					// conversion of square/attackbit to the corresponding 64 board RANK: 8-attackbit
					int file = FILES[square];
					int rank = 7 - attackbit;
					MG_FILE_ATTACK[square][state6Bit] |=  bitSet(BOARDINDEX[file][rank]);
				}
			}
		}
	}


	//  DIAGA8H1_ATTACKS attacks (BISHOPS and QUEENS):
	for ( tSquare square = square0; square < squareNumber; square++)
	{
		for (int state6Bit = 0; state6Bit < 64; state6Bit++)
		{
			for (tSquare attackbit = square0; attackbit < 8; attackbit++) // from LSB to MSB
			{
				//  conversion from 64 board squares to the 8 corresponding positions in the GEN_SLIDING_ATTACKS array: MIN((8-RANKS[square]),(FILES[square]-1))
				if (GEN_SLIDING_ATTACKS[(7-RANKS[square]) < (FILES[square]) ? (7-RANKS[square]) : (FILES[square])][state6Bit] & bitSet(attackbit))
				{
					int file;
					int rank;
					// the bit is set, so we need to update FILE_ATTACKS accordingly:
					// conversion of square/attackbit to the corresponding 64 board file and rank:
					int diaga8h1 = FILES[square] + RANKS[square]; // from 0 to 14, longest diagonal = 7
					if (diaga8h1 < 8)
					{
						file = attackbit;
						rank = diaga8h1 - file;
					}
					else
					{
						rank = 7 - attackbit;
						file = diaga8h1 - rank;
					}
					if ((file >= 0) && (file < 8) && (rank >= 0) && (rank < 8))
					{
						MG_DIAGA8H1_ATTACK[square][state6Bit] |=  bitSet(BOARDINDEX[file][rank]);
					}
				}
			}
		}
	}

	//  DIAGA1H8_ATTACKS attacks (BISHOPS and QUEENS):
	for (int square = 0; square < 64; square++)
	{
		for (int state6Bit = 0; state6Bit < 64; state6Bit++)
		{
			for (tSquare attackbit = square0; attackbit < 8; attackbit++) // from LSB to MSB
			{
				//  conversion from 64 board squares to the 8 corresponding positions in the GEN_SLIDING_ATTACKS array: MIN((8-RANKS[square]),(FILES[square]-1))
				if (GEN_SLIDING_ATTACKS[(RANKS[square]) < (FILES[square]) ? (RANKS[square]) : (FILES[square])][state6Bit] & bitSet(attackbit))
				{
					int file;
					int rank;
					// the bit is set, so we need to update FILE_ATTACKS accordingly:
					// conversion of square/attackbit to the corresponding 64 board file and rank:
					int diaga1h8 = FILES[square] - RANKS[square]; // from -7 to 7, longest diagonal = 0
					if (diaga1h8 < 0)
					{
						file = attackbit;
						rank = file - diaga1h8;
					}
					else
					{
						rank = attackbit ;
						file = diaga1h8 + rank;
					}
					if ((file >= 0) && (file < 8) && (rank >= 0) && (rank < 8))
					{
						MG_DIAGA1H8_ATTACK[square][state6Bit] |=  bitSet(BOARDINDEX[file][rank]);
					}
				}
			}
		}
	}




	for (unsigned int square = 0; square < 64; square++){
		bitMap x=0;
		ROOK_PSEUDO_ATTACK[square]=attackFromRook((tSquare)square,x);
		BISHOP_PSEUDO_ATTACK[square]=attackFromBishop((tSquare)square,x);
	}
}



template<Movegen::genType type>
void Movegen::generateMoves(Position& p){

	Position::state &s =p.getActualState();
	Position::bitboardIndex piece  =(Position::bitboardIndex)(s.nextMove+Position::whiteKing);



	bitMap thirdRankMask = RANKMASK[!s.nextMove? A3:A6];
	bitMap seventhRankMask = RANKMASK[!s.nextMove? A7:A2];
	bitMap pawns =  s.Us[Position::Pawns]&~seventhRankMask;
	bitMap promotionPawns =  s.Us[Position::Pawns] &seventhRankMask;

	bitMap kingTarget;
	bitMap target;
	if(type==Movegen::allEvasionMg){
		target=(s.checkers | SQUARES_BETWEEN[p.pieceList[Position::whiteKing+s.nextMove][0]][firstOne(s.checkers)]) &~ s.Us[Position::Pieces];
		kingTarget= ~ s.Us[Position::Pieces];
	}else if(type== Movegen::allNonEvasionMg){
		target= ~ s.Us[Position::Pieces];
		kingTarget= ~ s.Us[Position::Pieces];
	}else if(type== Movegen::captureMg){
		target= s.Them[Position::Pieces];
		kingTarget = s.Them[Position::Pieces];
	}else if(type== Movegen::quietMg){
		target= ~p.bitBoard[Position::occupiedSquares];
		kingTarget= ~ p.bitBoard[Position::occupiedSquares];
	}else if(type== Movegen::quietChecksMg){
		target= ~p.bitBoard[Position::occupiedSquares];
		kingTarget= ~ p.bitBoard[Position::occupiedSquares];
	}
	bitMap enemy = s.Them[Position::Pieces];
	bitMap & occupiedSquares = p.bitBoard[Position::occupiedSquares];

	bitMap moves;
	Move m;
	m.packed=0;

	//------------------------------------------------------
	// king
	//------------------------------------------------------
	{
		tSquare from=p.pieceList[piece][0];
		m.from=from;
		moves= attackFromKing(from)& kingTarget;
		while (moves){
			m.to=firstOne(moves);
			if(!(p.getAttackersTo((tSquare)m.to,p.bitBoard[Position::occupiedSquares] & ~s.Us[Position::King]) & s.Them[Position::Pieces]))
			{
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;
		}
	}
	if(type==Movegen::allEvasionMg){
		if(moreThanOneBit(s.checkers)){
			return;
		}
	}


	piece= (Position::bitboardIndex)(piece+1);
	//------------------------------------------------------
	// queen
	//------------------------------------------------------
	for(unsigned int i=0;i<p.pieceCount[piece];i++){
		tSquare from=p.pieceList[piece][i];
		m.from=from;
		moves= attackFromRook(from,occupiedSquares);
		moves |= attackFromBishop(from,occupiedSquares);
		moves &=target;
		while (moves){

			m.to=firstOne(moves);
			if(!(s.pinnedPieces & bitSet(from)) ||
					squaresAligned(from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;

		}
	}


	piece= (Position::bitboardIndex)(piece+1);
	//------------------------------------------------------
	// rook
	//------------------------------------------------------
	for(unsigned int i=0;i<p.pieceCount[piece];i++){
		tSquare from=p.pieceList[piece][i];
		m.from=from;
		moves= attackFromRook(from,occupiedSquares);
		moves &=target;
		while (moves){
			m.to=firstOne(moves);
			if(!(s.pinnedPieces & bitSet(from)) ||
					squaresAligned(from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}

			moves &= moves-1;
		}

	}


	piece= (Position::bitboardIndex)(piece+1);
	//------------------------------------------------------
	// bishop
	//------------------------------------------------------
	for(unsigned int i=0;i<p.pieceCount[piece];i++){
		tSquare from=p.pieceList[piece][i];
		m.from=from;
		moves= attackFromBishop(from,occupiedSquares);
		moves &=target;
		while (moves){
			m.to=firstOne(moves);
			if(!(s.pinnedPieces & bitSet(from)) ||
				squaresAligned(from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;
		}
	}



	piece= (Position::bitboardIndex)(piece+1);
	//------------------------------------------------------
	// knight
	//------------------------------------------------------
	for(unsigned int i=0;i<p.pieceCount[piece];i++){
		tSquare from=p.pieceList[piece][i];
		m.from=from;
		if(!(s.pinnedPieces & bitSet(from))){
			moves = attackFromKnight(p.pieceList[piece][i])& target;
			while (moves){
				m.to=firstOne(moves);
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
				moves &= moves-1;

			}
		}
	}



	piece= (Position::bitboardIndex)(piece+1);
	//------------------------------------------------------
	// Pawns
	//------------------------------------------------------

	if(type!= Movegen::captureMg){
		bitMap pawnPushed;
		//push
		moves=(s.nextMove? (pawns>>8):(pawns<<8))&~occupiedSquares;
		pawnPushed=moves;
		moves &=target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-pawnPush(s.nextMove);
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;
		}
		//double push
		moves=(s.nextMove? ((pawnPushed&thirdRankMask)>>8):((pawnPushed&thirdRankMask)<<8))&~occupiedSquares& target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-2*pawnPush(s.nextMove);
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;
		}
	}

	int delta;
	if(type!= Movegen::quietMg && type!=Movegen::quietChecksMg){
		//left capture
		delta=s.nextMove?-9:7;
		moves = (s.nextMove?(pawns&(~FILEMASK[A1]))>>9:(pawns&(~FILEMASK[A1]))<<7)&enemy& target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-delta;
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				//moveList.push_back(m);
				moveList[moveListSize++].packed=m.packed;
			}
			moves &= moves-1;
		}

		//right capture
		delta=s.nextMove?-7:9;
		moves = (s.nextMove?(pawns&(~FILEMASK[H1]))>>7:(pawns&(~FILEMASK[H1]))<<9)&enemy& target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-delta;
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				//moveList.push_back(m);
				moveList[moveListSize++].packed=m.packed;
			}
			moves &= moves-1;
		}
	}

	// PROMOTIONS
	if(type!= Movegen::captureMg){
		moves=(s.nextMove? (promotionPawns>>8):(promotionPawns<<8))&~occupiedSquares;

		moves &=target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-pawnPush(s.nextMove);
			m.flags=Move::fpromotion;
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1)){
					m.promotion=prom;
					if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
						//moveList.push_back(m);
						moveList[moveListSize++].packed=m.packed;
					}
				}
			}
			moves &= moves-1;
		}
	}

	int color = s.nextMove?1:0;
	if(type!= Movegen::quietMg && type!= Movegen::quietChecksMg){
		//left capture
		delta=s.nextMove?-9:7;
		moves = (s.nextMove?(promotionPawns&(~FILEMASK[A1]))>>9:(promotionPawns&(~FILEMASK[A1]))<<7)&enemy& target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-delta;
			m.flags=Move::fpromotion;
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1)){
					m.promotion=prom;
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;
		}

		//right capture
		delta=s.nextMove?-7:9;
		moves = (s.nextMove?(promotionPawns&(~FILEMASK[H1]))>>7:(promotionPawns&(~FILEMASK[H1]))<<9)&enemy& target;
		while(moves){
			m.to=firstOne(moves);
			m.from=m.to-delta;
			m.flags=Move::fpromotion;
			if(!(s.pinnedPieces & bitSet((tSquare)m.from)) ||
				squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))
			{
				for(Move::epromotion prom=Move::promQueen;prom<= Move::promKnight; prom=(Move::epromotion)(prom+1)){
					m.promotion=prom;
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
			}
			moves &= moves-1;
		}
		m.promotion=0;

		// ep capture

		if(s.epSquare!=squareNone){

			tSquare kingSquare =p.pieceList[Position::whiteKing+s.nextMove][0];
			bitMap epAttacker=pawns & attackFromPawn(s.epSquare,1-color);
			while(epAttacker){
				tSquare from=firstOne(epAttacker);
				bitMap captureSquare= FILEMASK[s.epSquare] & RANKMASK[from];
				bitMap occ= occupiedSquares^bitSet(from)^bitSet(s.epSquare)^captureSquare;

				if(	!((attackFromRook(kingSquare, occ) & (s.Them[Position::Queens] |s.Them[Position::Rooks]))|
						(Movegen::attackFromBishop(kingSquare, occ) & (s.Them[Position::Queens] |s.Them[Position::Bishops])))
				){
					m.to=s.epSquare;
					m.from=from;
					m.flags=Move::fenpassant;
					//moveList.push_back(m);
					moveList[moveListSize++].packed=m.packed;
				}
				epAttacker &=epAttacker-1;
			}

		}
	}


	//king castle
	if(type !=Movegen::allEvasionMg && type!= Movegen::captureMg){

		if(s.castleRights & ((Position::wCastleOO |Position::wCastleOOO)<<(2*color))){


			if((s.castleRights &((Position::wCastleOO)<<(2*color))) &&!s.checkers &&!(castlePath[color][KING_SIDE_CASTLE] & p.bitBoard[Position::occupiedSquares])){
				tSquare kingSquare=p.pieceList[Position::whiteKing+s.nextMove][0];
				bool castleDenied=false;
				for( tSquare x=(tSquare)1;x<3;x++){
					if(p.getAttackersTo(kingSquare+x,p.bitBoard[Position::occupiedSquares]) & s.Them[Position::Pieces]){
						castleDenied=true;
						break;
					}
				}
				if(!castleDenied){
					m.flags=Move::fcastle;
					m.from=kingSquare;
					m.to=kingSquare+2;
					if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
						//moveList.push_back(m);
						moveList[moveListSize++].packed=m.packed;
					}
				}


			}
			if((s.castleRights &((Position::wCastleOOO)<<(2*color))) &&!s.checkers && !(castlePath[color][QUEEN_SIDE_CASTLE] & p.bitBoard[Position::occupiedSquares])){
				tSquare kingSquare=p.pieceList[Position::whiteKing+s.nextMove][0];
				bool castleDenied=false;
				for( tSquare x=(tSquare)1;x<3;x++){
					if(p.getAttackersTo(kingSquare-x,p.bitBoard[Position::occupiedSquares]) & s.Them[Position::Pieces]){
						castleDenied=true;
						break;
					}
				}
				if(!castleDenied){
					m.flags=Move::fcastle;
					m.from=kingSquare;
					m.to=kingSquare-2;
					if(type !=Movegen::quietChecksMg || p.moveGivesCheck(m)){
						//moveList.push_back(m);
						moveList[moveListSize++].packed=m.packed;
					}
				}
			}
		}
	}
	assert(moveListSize<=MAX_MOVE_PER_POSITION);



}
template void Movegen::generateMoves<Movegen::captureMg>(Position& p);
template void Movegen::generateMoves<Movegen::quietMg>(Position& p);


template<>
void Movegen::generateMoves<Movegen::allMg>(Position& p){
	/*Move m;
	moveListIndex=0;
	for (int i=0;i<65535;i++){
		m.packed=i;
		if(isMoveLegal(p,m)){
			moveList[moveListIndex].packed=m.packed;
			//sync_cout<<p.displayUci(m)<<sync_endl;
			moveListIndex++;
		}
	}*/
	Position::state &s =p.getActualState();
	if(s.checkers){
		generateMoves<Movegen::allEvasionMg>(p);
	}
	else{
		generateMoves<Movegen::genType::captureMg>(p);
		generateMoves<Movegen::genType::quietMg>(p);
		//generateMoves<Movegen::allNonEvasionMg>(p);
	}

}

bool Movegen::isMoveLegal(Position&p, Move m){
	Position::state &s =p.getActualState();
	Position::bitboardIndex piece=p.board[m.from];
	// pezzo inesistente
	if(piece==Position::empty){
		p.display();
		sync_cout<<p.displayUci(m)<<": empty square"<<sync_endl;
		while(1){}
		return false;
	}
	// pezzo del colore sbagliato
	if(s.nextMove?!p.isblack(piece):p.isblack(piece)){
		p.display();
		sync_cout<<p.displayUci(m)<<": wrong color"<<sync_endl;
		while(1){}
		return false;
	}

	//casa di destinazione irraggiungibile
	if(bitSet((tSquare)m.to) & s.Us[Position::Pieces]){
		p.display();
		sync_cout<<p.displayUci(m)<<": occupied square"<<sync_endl;
		while(1){}

		return false;
	}
	//scacco
	if(s.checkers){
		if( moreThanOneBit(s.checkers)){ //scacco doppio posso solo muovere il re
			if(!p.isKing(piece)){
				p.display();
				sync_cout<<p.displayUci(m)<<": double check"<<sync_endl;
				while(1){}
				return false;
			}
		}
		else{
			// scacco singolo i pezzi che non sono re possono catturare il pezzo attaccante oppure mettersi nel mezzo
			if(
				!p.isKing(piece)
				&& !(
					((bitSet((tSquare)(m.to-(m.flags==Move::fenpassant?pawnPush(s.nextMove):0)))) & s.checkers)
					|| ((bitSet((tSquare)m.to) & SQUARES_BETWEEN[p.pieceList[Position::whiteKing+s.nextMove][0]][firstOne(s.checkers)]) &~ s.Us[Position::Pieces])
				)
			){
				p.display();
				sync_cout<<p.displayUci(m)<<": single check"<<sync_endl;
				while(1){}
				return false;
			}
		}
	}
	if((s.pinnedPieces & bitSet((tSquare)m.from) && !squaresAligned((tSquare)m.from,(tSquare)m.to,p.pieceList[Position::whiteKing+s.nextMove][0]))){
		p.display();
		sync_cout<<p.displayUci(m)<<": pinned problem"<<sync_endl;
		while(1){}
		return false;
	}


	// promozione impossibile!!
	if(m.flags==Move::fpromotion && ((RANKS[m.from]!=(s.nextMove?1:6)) || !(p.isPawn(piece)))){
		p.display();
		sync_cout<<p.displayUci(m)<<": promotion flag"<<sync_endl;
		while(1){}
		return false;
	}

	if(m.flags!=Move::fpromotion && m.promotion!=0){
			p.display();
			sync_cout<<p.displayUci(m)<<": promotion2 flag"<<sync_endl;
			while(1){}
			return false;
		}
	//arrocco impossibile
	if(m.flags==Move::fcastle){
		if(!p.isKing(piece) || (FILES[m.from]!=FILES[E1]) || (abs(m.from-m.to)!=2 ) || (RANKS[m.from]!=RANKS[A1] && RANKS[m.from]!=RANKS[A8])){
			p.display();
			sync_cout<<p.displayUci(m)<<": castle impossibile"<<sync_endl;
			while(1){}
			return false;
		}
	}
	//en passant impossibile
	if(m.flags==Move::fenpassant && (!p.isPawn(piece) || ((tSquare)m.to)!=s.epSquare)){
		p.display();
		sync_cout<<p.displayUci(m)<<": enpassant error"<<sync_endl;
		while(1){}
		return false;
	}
	//en passant impossibile
	if(m.flags!=Move::fenpassant && p.isPawn(piece) && ((tSquare)m.to==s.epSquare)){
		p.display();
		sync_cout<<p.displayUci(m)<<": enpassant error"<<sync_endl;
		while(1){}
		return false;
	}




	switch(piece){
		case Position::whiteKing:
		case Position::blackKing:
		{
			if(m.flags== Move::fcastle){
				int color = s.nextMove?1:0;
				if(!(s.castleRights &  bitSet((tSquare)(((m.from-m.to)>0)+2*color)))
					|| castlePath[color][(m.from-m.to)>0] & p.bitBoard[Position::occupiedSquares]
				){
					p.display();
					sync_cout<<p.displayUci(m)<<": king castle error"<<sync_endl;
					while(1){}
					return false;
				}
				if(m.to>m.from){
					for(tSquare x=(tSquare)m.from;x<=(tSquare)m.to ;x++){
						if(p.getAttackersTo(x,p.bitBoard[Position::occupiedSquares] & ~s.Us[Position::King]) & s.Them[Position::Pieces]){
							return false;
						}
					}
				}else{
					for(tSquare x=(tSquare)m.to;x<=(tSquare)m.from ;x++){
						if(p.getAttackersTo(x,p.bitBoard[Position::occupiedSquares] & ~s.Us[Position::King]) & s.Them[Position::Pieces]){
							return false;
						}
					}
				}
			}
			else{
				if(!(attackFromKing((tSquare)m.from) &bitSet((tSquare)m.to)) || (bitSet((tSquare)m.to)&s.Us[Position::Pieces])){
					p.display();
					sync_cout<<p.displayUci(m)<<": king move"<<sync_endl;
					while(1){}
					return false;
				}
				//king moves should not leave king in check
				if((p.getAttackersTo((tSquare)m.to,p.bitBoard[Position::occupiedSquares] & ~s.Us[Position::King]) & s.Them[Position::Pieces])){
					p.display();
					sync_cout<<p.displayUci(m)<<": king leaved on chess"<<sync_endl;
					while(1){}
					return false;
				}
			}





		}
			break;

		case Position::whiteRooks:
		case Position::blackRooks:
			if(!(getRookPseudoAttack((tSquare)m.from) & bitSet((tSquare)m.to)) || !(attackFromRook((tSquare)m.from,p.bitBoard[Position::occupiedSquares])& bitSet((tSquare)m.to))){
				p.display();
				sync_cout<<p.displayUci(m)<<": rook problem"<<sync_endl;
				while(1){}
				return false;
			}
			break;

		case Position::whiteQueens:
		case Position::blackQueens:
			if(
				!(
					(getBishopPseudoAttack((tSquare)m.from) | getRookPseudoAttack((tSquare)m.from))
					& bitSet((tSquare)m.to)
				)
				||
				!(
					(
						attackFromBishop((tSquare)m.from,p.bitBoard[Position::occupiedSquares])
						|attackFromRook((tSquare)m.from,p.bitBoard[Position::occupiedSquares])
					)
					& bitSet((tSquare)m.to)
				)
			){
				p.display();
				sync_cout<<p.displayUci(m)<<": queen problem"<<sync_endl;
				while(1){}
				return false;
			}
			break;

		case Position::whiteBishops:
		case Position::blackBishops:
			if(!(getBishopPseudoAttack((tSquare)m.from) & bitSet((tSquare)m.to)) || !(attackFromBishop((tSquare)m.from,p.bitBoard[Position::occupiedSquares])& bitSet((tSquare)m.to))){
				p.display();
				sync_cout<<p.displayUci(m)<<": bishop flag"<<sync_endl;
				while(1){}
				return false;
			}
			break;

		case Position::whiteKnights:
		case Position::blackKnights:
			if(!(attackFromKnight((tSquare)m.from)& bitSet((tSquare)m.to))){
				p.display();
				sync_cout<<p.displayUci(m)<<": knight"<<sync_endl;
				while(1){}
				return false;
			}

			break;

			// todo gestire caso particolare en passant. re va sotto scacco dopo aver fatto enpassant
		case Position::whitePawns:

			if(
				// not valid pawn push
				(m.from+pawnPush(s.nextMove)!= m.to || (bitSet((tSquare)m.to)&p.bitBoard[Position::occupiedSquares]))
				// not valid pawn double push
				&& ((m.from+2*pawnPush(s.nextMove)!= m.to) || (RANKS[m.from]!=1) || ((bitSet((tSquare)m.to) | bitSet((tSquare)(m.to-8)))&p.bitBoard[Position::occupiedSquares]))
				// not valid pawn attack
				&& (!(attackFromPawn((tSquare)m.from,s.nextMove!=Position::whiteTurn)&bitSet((tSquare)m.to)) || !((bitSet((tSquare)m.to)) &(s.Them[Position::Pieces]|bitSet(s.epSquare))))
			){
				p.display();
				sync_cout<<p.displayUci(m)<<": pawn push"<<sync_endl;
				while(1){}
				return false;
			}
			if(RANKS[m.from]==6 && m.flags!=Move::fpromotion){
				p.display();
				sync_cout<<p.displayUci(m)<<": pawn push"<<sync_endl;
				while(1){}
				return false;

			}
			if(m.flags== Move::fenpassant){
				bitMap captureSquare= FILEMASK[s.epSquare] & RANKMASK[m.from];
				bitMap occ= p.bitBoard[Position::occupiedSquares]^bitSet((tSquare)m.from)^bitSet(s.epSquare)^captureSquare;
				tSquare kingSquare=p.pieceList[Position::whiteKing+s.nextMove][0];
				if((attackFromRook(kingSquare, occ) & (s.Them[Position::Queens] |s.Them[Position::Rooks]))|
							(attackFromBishop(kingSquare, occ) & (s.Them[Position::Queens] |s.Them[Position::Bishops]))){
				p.display();
				sync_cout<<p.displayUci(m)<<": pawn push"<<sync_endl;
				while(1){}
				return false;
				}
			}

			break;
		case Position::blackPawns:
			if(
				// not valid pawn push
				(m.from+pawnPush(s.nextMove)!= m.to || (bitSet((tSquare)m.to)&p.bitBoard[Position::occupiedSquares]))
				// not valid pawn double push
				&& ((m.from+2*pawnPush(s.nextMove)!= m.to) || (RANKS[m.from]!=6) || ((bitSet((tSquare)m.to) | bitSet((tSquare)(m.to+8)))&p.bitBoard[Position::occupiedSquares]))
				// not valid pawn attack
				&& (!(attackFromPawn((tSquare)m.from,s.nextMove!=Position::whiteTurn)&bitSet((tSquare)m.to)) || !((bitSet((tSquare)m.to)) &(s.Them[Position::Pieces]| bitSet(s.epSquare))))
			){
				p.display();
				sync_cout<<p.displayUci(m)<<": pawn push"<<sync_endl;
				while(1){}
				return false;
			}

			if(RANKS[m.from]==1 && m.flags!=Move::fpromotion){
				p.display();
				sync_cout<<p.displayUci(m)<<": pawn push"<<sync_endl;
				while(1){}
				return false;

			}
			if(m.flags== Move::fenpassant){
				bitMap captureSquare= FILEMASK[s.epSquare] & RANKMASK[m.from];
				bitMap occ= p.bitBoard[Position::occupiedSquares]^bitSet((tSquare)m.from)^bitSet(s.epSquare)^captureSquare;
				tSquare kingSquare=p.pieceList[Position::whiteKing+s.nextMove][0];
				if((attackFromRook(kingSquare, occ) & (s.Them[Position::Queens] |s.Them[Position::Rooks]))|
							(attackFromBishop(kingSquare, occ) & (s.Them[Position::Queens] |s.Them[Position::Bishops]))){
				p.display();
				sync_cout<<p.displayUci(m)<<": pawn push"<<sync_endl;
				while(1){}
				return false;
				}
			}
			break;
		default:
			return false;

	}


	return true;
}








Move & Movegen::getNextMove(){

	bool found=false;

	while(!found){
		switch(stagedGeneratorState){
		case generateCaptureMoves:
			generateMoves<Movegen::genType::captureMg>(pos);
			stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);
			break;
		case generateQuietMoves:
			generateMoves<Movegen::genType::quietMg>(pos);
			stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);
			break;
		case generateEvasionMoves:
			generateMoves<Movegen::allEvasionMg>(pos);
			stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);
			break;
		case generateQuiescentMoves:
			generateMoves<Movegen::captureMg>(pos);
			stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);
			break;

		case iterateEvasionMoves:
		case iterateQuietMoves:
			if(moveListPosition<moveListSize){
				return moveList[moveListPosition++];
			}
			else{
				return NOMOVE;
			}
			break;
		case iterateCaptureMoves:
		case iterateQuiescentMoves:
		{
			if(moveListPosition<moveListSize){
				Score bestScore=-SCORE_INFINITE;
				unsigned int bestIndex=moveListPosition;
				for(unsigned int i=moveListPosition;i<moveListSize;i++){ // itera sulle mosse rimanenti
					Score res=pos.getMvvLvaScore(moveList[i]);
					if(res>bestScore){
						bestScore=res;
						bestIndex=i;
					}
				}

				if(bestIndex!=moveListPosition){
					std::swap(moveList[moveListPosition].packed,moveList[bestIndex].packed);
				}
				return moveList[moveListPosition++];
			}
			stagedGeneratorState=(eStagedGeneratorState)(stagedGeneratorState+1);

		}
			break;
		default:
			return NOMOVE;

			break;


		}
	}

	return NOMOVE;


}
