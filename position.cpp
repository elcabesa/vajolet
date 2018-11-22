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
#include <cstdlib>
#include <sstream>

#include "io.h"
#include "movegen.h"
#include "parameters.h"
#include "position.h"
#include "transposition.h"
#include "vajolet.h"

/* PST data */
const int Center[8]	= { -3, -1, +0, +1, +1, +0, -1, -3};
const int KFile[8]	= { +3, +4, +2, +0, +0, +2, +4, +3};
const int KRank[8]	= { +1, +0, -2, -3, -4, -5, -6, -7};

simdScore Position::pieceValue[lastBitboard];
simdScore Position::pstValue[lastBitboard][squareNumber];
simdScore Position::nonPawnValue[lastBitboard];
Position::eCastle Position::castleRightsMask[squareNumber];

bool Position::perftUseHash = false;


void Position::initPstValues(void)
{
	for(int piece = 0; piece < lastBitboard; piece++)
	{
		for(tSquare s = (tSquare)0; s < squareNumber; s++)
		{
			assert(s<squareNumber);
			nonPawnValue[piece] = simdScore{0,0,0,0};
			pstValue[piece][s] = simdScore{0,0,0,0};
			int rank = RANKS[s];
			int file = FILES[s];

			if(piece > occupiedSquares && piece < whitePieces )
			{

				if(piece == Pawns)
				{
					pstValue[piece][s] = simdScore{0,0,0,0};
					if(s==D3)
					{
						pstValue[piece][s] = PawnD3;
					}
					if(s==D4)
					{
						pstValue[piece][s] = PawnD4;
					}
					if(s==D5)
					{
						pstValue[piece][s] = PawnD5;
					}
					if(s==E3)
					{
						pstValue[piece][s] = PawnE3;
					}
					if(s==E4)
					{
						pstValue[piece][s] = PawnE4;
					}
					if(s==E5)
					{
						pstValue[piece][s] = PawnE5;
					}
					pstValue[piece][s] += PawnRankBonus * (rank - 2);
					pstValue[piece][s] += Center[file] * PawnCentering;
				}
				if(piece == Knights)
				{
					pstValue[piece][s] = KnightPST * (Center[file] + Center[rank]);
					if(rank==0)
					{
						pstValue[piece][s] -= KnightBackRankOpening;
					}
				}
				if(piece == Bishops)
				{
					pstValue[piece][s] = BishopPST * (Center[file] + Center[rank]);
					if(rank==0)
					{
						pstValue[piece][s] -= BishopBackRankOpening;
					}
					if((file==rank) || (file+rank==7))
					{
						pstValue[piece][s] += BishopOnBigDiagonals;
					}
				}
				if(piece == Rooks)
				{
					pstValue[piece][s] = RookPST * (Center[file]);
					if(rank==0)
					{
						pstValue[piece][s] -= RookBackRankOpening;
					}
				}
				if(piece == Queens)
				{
					pstValue[piece][s] = QueenPST * (Center[file] + Center[rank]);
					if(rank==0)
					{
						pstValue[piece][s] -= QueenBackRankOpening;
					}
				}
				if(piece == King)
				{
					pstValue[piece][s] = simdScore{
							(KFile[file]+KRank[rank]) * KingPST[0],
							(Center[file]+Center[rank]) * KingPST[1],
							0,0};
				}
				if(!isKing((bitboardIndex)piece))
				{
					pstValue[piece][s] += pieceValue[piece];
				}

				if( !isPawn((bitboardIndex)piece) && !isKing((bitboardIndex)piece))
				{
					nonPawnValue[piece][0] = pieceValue[piece][0];
					nonPawnValue[piece][1] = pieceValue[piece][1];
				}

			}
			else if(piece >separationBitmap && piece <blackPieces )
			{
				int r = 7 - rank;
				//int f = 7 - file;
				int f = file;
				pstValue[piece][s] = -pstValue[ piece - separationBitmap ][BOARDINDEX[f][r]];

				if( !isPawn((bitboardIndex)piece) && !isKing((bitboardIndex)piece))
				{
					nonPawnValue[piece][2] = pieceValue[piece][0];
					nonPawnValue[piece][3] = pieceValue[piece][1];
				}
			}
			else{
				pstValue[piece][s] = simdScore{0,0,0,0};
			}
		}
	}

}


/*! \brief setup a position from a fen string
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::setupFromFen(const std::string& fenStr)
{
	char col,row,token;
	tSquare sq = A8;
	std::istringstream ss(fenStr);

	clear();
	ss >> std::noskipws;

	while ((ss >> token) && !std::isspace(token))
	{
		if (isdigit(token))
			sq += tSquare(token - '0'); // Advance the given number of files
		else if (token == '/')
			sq -= tSquare(16);
		else
		{
			switch (token)
			{
			case 'P':
				putPiece(whitePawns,sq);
				break;
			case 'N':
				putPiece(whiteKnights,sq);
				break;
			case 'B':
				putPiece(whiteBishops,sq);
				break;
			case 'R':
				putPiece(whiteRooks,sq);
				break;
			case 'Q':
				putPiece(whiteQueens,sq);
				break;
			case 'K':
				putPiece(whiteKing,sq);
				break;
			case 'p':
				putPiece(blackPawns,sq);
				break;
			case 'n':
				putPiece(blackKnights,sq);
				break;
			case 'b':
				putPiece(blackBishops,sq);
				break;
			case 'r':
				putPiece(blackRooks,sq);
				break;
			case 'q':
				putPiece(blackQueens,sq);
				break;
			case 'k':
				putPiece(blackKing,sq);
				break;
			}
			sq++;
		}
	}

	state &x= getActualState();


	ss >> token;
	x.setNextTurn( token == 'w' ? whiteTurn : blackTurn );

	updateUsThem();

	ss >> token;

	x.clearCastleRight();
	while ((ss >> token) && !isspace(token))
	{
		switch(token){
		case 'K':
			x.setCastleRight( wCastleOO );
			break;
		case 'Q':
			x.setCastleRight( wCastleOOO );
			break;
		case 'k':
			x.setCastleRight( bCastleOO );
			break;
		case 'q':
			x.setCastleRight( bCastleOOO );
			break;
		}
	}

	x.epSquare=squareNone;
	if (((ss >> col) && (col >= 'a' && col <= 'h'))
		&& ((ss >> row) && (row == '3' || row == '6')))
	{
		x.epSquare =(tSquare) (((int) col - 'a') + 8 * (row - '1')) ;
		if (!(getAttackersTo(x.epSquare) & bitBoard[ x.getPawnsOfActivePlayer() ]))
			x.epSquare = squareNone;
	}



	ss >> std::skipws >> x.fiftyMoveCnt;
	if(ss.eof()){
		ply = int( x.isBlackTurn() );
		x.fiftyMoveCnt=0;

	}else{
		ss>> ply;
		ply = std::max(2 * (ply - 1), (unsigned int)0) + int( x.isBlackTurn() );
	}

	x.pliesFromNull = 0;
	x.setCurrentMove( Move::NOMOVE );
	x.capturedPiece = empty;

	x.setMaterialValue( calcMaterialValue() );
	x.setNonPawnValue( calcNonPawnMaterialValue() );

	x.key=calcKey();
	x.pawnKey=calcPawnKey();
	x.materialKey=calcMaterialKey();

	calcCheckingSquares();

	x.setHiddenCheckers( getHiddenCheckers<true>() );
	x.setPinnedPieces( getHiddenCheckers<false>() );
	x.setCheckers( getAttackersTo(getSquareOfThePiece( x.getKingOfActivePlayer() ) ) & bitBoard[x.getPiecesOfOtherPlayer()] );

	checkPosConsistency(1);
}

void Position::setup(const std::string& code, Color c)
{

  assert(code.length() > 0 && code.length() < 8);
  assert(code[0] == 'K');

  std::string sides[] = { code.substr(code.find('K', 1)),      // Weak
                     code.substr(0, code.find('K', 1)) }; // Strong

  std::transform(sides[c].begin(), sides[c].end(), sides[c].begin(), tolower);

  std::string fenStr =  sides[0] + char(8 - sides[0].length() + '0') + "/8/8/8/8/8/8/"
                 + sides[1] + char(8 - sides[1].length() + '0') + " w - - 0 10";

  sync_cout<<fenStr<<sync_endl;
  return setupFromFen(fenStr);
}




/*! \brief init the score value in the static const
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::initScoreValues(void)
{
	for(auto &val :pieceValue)
	{
		val = simdScore{0,0,0,0};
	}
	pieceValue[whitePawns] = initialPieceValue[whitePawns];
	pieceValue[whiteKnights] = initialPieceValue[whiteKnights];
	pieceValue[whiteBishops] = initialPieceValue[whiteBishops];
	pieceValue[whiteRooks] = initialPieceValue[whiteRooks];
	pieceValue[whiteQueens] = initialPieceValue[whiteQueens];
	pieceValue[whiteKing] = initialPieceValue[whiteKing];

	pieceValue[blackPawns] = pieceValue[whitePawns];
	pieceValue[blackKnights] = pieceValue[whiteKnights];
	pieceValue[blackBishops] = pieceValue[whiteBishops];
	pieceValue[blackRooks] = pieceValue[whiteRooks];
	pieceValue[blackQueens] = pieceValue[whiteQueens];
	pieceValue[blackKing] = pieceValue[whiteKing];

	initPstValues();
}
/*! \brief clear a position and his history
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::clear()
{
	for (tSquare i = square0; i < squareNumber; i++)
	{
		squares[i] = empty;
	};
	for (int i = 0; i < lastBitboard; i++)
	{
		bitBoard[i] = 0;
	}
	stateInfo.clear();
	stateInfo.emplace_back(state());

}


/*	\brief display a board for debug purpose
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
void Position::display()const
{
	sync_cout<<getFen()<<sync_endl;

	int rank, file;
	const state& st =getActualStateConst();
	sync_cout;
	{
		for (rank = 7; rank >= 0; rank--)
		{
			std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
			std::cout << rank+1 <<  " |";
			for (file = 0; file <= 7; file++)
			{
				std::cout << " " << getPieceName(squares[BOARDINDEX[file][rank]] ) << " |";
			}
			std::cout << std::endl;
		}
		std::cout << "  +---+---+---+---+---+---+---+---+" << std::endl;
		std::cout << "    a   b   c   d   e   f   g   h" << std::endl << std::endl;

	}
	std::cout <<(st.isBlackTurn() ? "BLACK TO MOVE" : "WHITE TO MOVE" ) <<std::endl;
	std::cout <<"50 move counter "<<st.fiftyMoveCnt<<std::endl;
	std::cout <<"castleRights ";
	if( st.hasCastleRight(wCastleOO) ) std::cout<<"K";
	if( st.hasCastleRight(wCastleOOO) ) std::cout<<"Q";
	if( st.hasCastleRight(bCastleOO) ) std::cout<<"k";
	if( st.hasCastleRight(bCastleOOO) ) std::cout<<"q";
	if( !st.hasCastleRights() ) std::cout<<"-";
	std::cout<<std::endl;
	std::cout <<"epsquare ";

	if(st.epSquare!=squareNone){
		std::cout<<char('a'+FILES[st.epSquare]);
		std::cout<<char('1'+RANKS[st.epSquare]);
	}else{
		std::cout<<'-';
	}
	std::cout<<std::endl;
	std::cout<<"material "<<st.getMaterialValue()[0]/10000.0<<std::endl;
	std::cout<<"white material "<<st.getNonPawnValue()[0]/10000.0<<std::endl;
	std::cout<<"black material "<<st.getNonPawnValue()[2]/10000.0<<std::endl;

	std::cout<<sync_endl;

}


/*	\brief display the fen string of the position
	\author Marco Belli
	\version 1.0
	\date 22/10/2013
*/
std::string  Position::getFen() const {

	std::string s;

	int file,rank;
	int emptyFiles = 0;
	const state& st = getActualStateConst();
	for (rank = 7; rank >= 0; rank--)
	{
		emptyFiles = 0;
		for (file = 0; file <= 7; file++)
		{
			if(squares[BOARDINDEX[file][rank]] != 0)
			{
				if(emptyFiles!=0)
				{
					s+=std::to_string(emptyFiles);
				}
				emptyFiles=0;
				s += getPieceName( squares[BOARDINDEX[file][rank]] );
			}
			else
			{
				emptyFiles++;
			}
		}
		if(emptyFiles!=0)
		{
			s += std::to_string(emptyFiles);
		}
		if(rank != 0)
		{
			s += '/';
		}
	}
	s += ' ';
	if(st.isBlackTurn() )
	{
		s += 'b';
	}
	else
	{
		s += 'w';
	}
	s += ' ';

	if( st.hasCastleRight(wCastleOO) )
	{
		s += "K";
	}
	if( st.hasCastleRight(wCastleOOO) )
	{
		s += "Q";
	}
	if( st.hasCastleRight(bCastleOO) )
	{
		s += "k";
	}
	if( st.hasCastleRight(bCastleOOO) )
	{
		s += "q";
	}
	if( !st.hasCastleRights() ){
		s += "-";
	}
	s += ' ';
	if(st.epSquare != squareNone)
	{
		s += char('a'+FILES[st.epSquare]);
		s += char('1'+RANKS[st.epSquare]);
	}
	else
	{
		s += '-';
	}
	s += ' ';
	s += std::to_string(st.fiftyMoveCnt);
	s += " " + std::to_string(1 + ( ply - int( st.isBlackTurn() ) ) / 2);


	return s;
}


std::string Position::getSymmetricFen() const {

	std::string s;
	int file,rank;
	int emptyFiles=0;
	const state& st =getActualStateConst();
	for (rank = 0; rank <=7 ; rank++)
	{
		emptyFiles=0;
		for (file = 0; file <=7; file++)
		{
			if(squares[BOARDINDEX[file][rank]]!=0)
			{
				if(emptyFiles!=0)
				{
					s += std::to_string(emptyFiles);
				}
				emptyFiles = 0;
				bitboardIndex xx = squares[BOARDINDEX[file][rank]];
				if(xx >= separationBitmap)
				{
					xx = (bitboardIndex)(xx - separationBitmap);
				}
				else
				{
					xx = (bitboardIndex)(xx + separationBitmap);
				}
				s += getPieceName( xx );
			}
			else
			{
				emptyFiles++;
			}
		}
		if(emptyFiles!=0){
			s += std::to_string(emptyFiles);
		}
		if(rank!=7)
		{
			s += '/';
		}
	}
	s += ' ';
	if( st.isBlackTurn() )
	{
		s += 'w';
	}
	else
	{
		s += 'b';
	}
	s += ' ';

	if( st.hasCastleRight(wCastleOO) )
	{
		s += "k";
	}
	if( st.hasCastleRight(wCastleOOO) )
	{
		s += "q";
	}
	if( st.hasCastleRight(bCastleOO) )
	{
		s += "K";
	}
	if( st.hasCastleRight(bCastleOOO) )
	{
		s += "Q";
	}
	if( !st.hasCastleRights() )
	{
		s += "-";
	}
	s += ' ';
	if(st.epSquare!=squareNone)
	{
		s += char('a'+FILES[st.epSquare]);
		s += char('1'+RANKS[st.epSquare]);
	}
	else
	{
		s += '-';
	}
	s += ' ';
	s += std::to_string(st.fiftyMoveCnt);
	s += " " + std::to_string(1 + ( ply - int( st.isBlackTurn() ) ) / 2);

	return s;
}

/*! \brief calc the hash key of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
uint64_t Position::calcKey(void) const
{
	uint64_t hash = 0;
	const state& st =getActualStateConst();

	for (int i = 0; i < squareNumber; i++)
	{
		if(squares[i]!=empty)
		{
			hash ^=HashKeys::keys[i][squares[i]];
		}
	}

	if( st.isBlackTurn() )
	{
		hash ^= HashKeys::side;
	}
	hash ^= HashKeys::castlingRight[st.getCastleRights()];


	if(st.epSquare != squareNone)
	{
		hash ^= HashKeys::ep[st.epSquare];
	}

	return hash;
}

/*! \brief calc the hash key of the pawn formation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
uint64_t Position::calcPawnKey(void) const
{
	uint64_t hash = 1;
	bitMap b= getBitmap(whitePawns);
	while(b)
	{
		tSquare n = iterateBit(b);
		hash ^= HashKeys::keys[n][whitePawns];
	}
	b= getBitmap(blackPawns);
	while(b)
	{
		tSquare n = iterateBit(b);
		hash ^= HashKeys::keys[n][blackPawns];
	}

	return hash;
}

/*! \brief calc the hash key of the material signature
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
uint64_t Position::calcMaterialKey(void) const
{
	uint64_t hash = 0;
	for (int i = whiteKing; i < lastBitboard; i++)
	{
		if ( i != occupiedSquares && i != whitePieces && i != blackPieces)
		{
			for (unsigned int cnt = 0; cnt < getPieceCount((bitboardIndex)i); cnt++)
			{
				hash ^= HashKeys::keys[i][cnt];
			}
		}
	}

	return hash;
}

/*! \brief calc the material value of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
simdScore Position::calcMaterialValue(void) const{
	simdScore score = {0,0,0,0};
	bitMap b= getOccupationBitmap();
	while(b)
	{
		tSquare s = iterateBit(b);
		bitboardIndex val = squares[s];
		score += pstValue[val][s];
	}
	return score;

}
/*! \brief calc the non pawn material value of the position
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
simdScore Position::calcNonPawnMaterialValue() const
{

	simdScore t[2] ={{0,0,0,0},{0,0,0,0}};
	simdScore res;

	bitMap b= getOccupationBitmap();
	while(b)
	{
		tSquare n = iterateBit(b);
		bitboardIndex val = squares[n];
		if(!isPawn(val) && !isKing(val) )
		{
			if(val > separationBitmap)
			{
				t[1] += pieceValue[val];
			}
			else{
				t[0] += pieceValue[val];
			}
		}
	}
	res = simdScore{t[0][0],t[0][1],t[1][0],t[1][1]};
	return res;

}
/*! \brief do a null move
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::doNullMove(void)
{

	insertState(getActualState());
	state &x = getActualState();

	x.setCurrentMove( Move::NOMOVE );
	if(x.epSquare != squareNone)
	{
		assert(x.epSquare<squareNumber);
		x.key ^= HashKeys::ep[x.epSquare];
		x.epSquare = squareNone;
	}
	x.key ^= HashKeys::side;
	x.fiftyMoveCnt++;
	x.pliesFromNull = 0;
	x.changeNextTurn();


	++ply;
	x.capturedPiece = empty;

	std::swap(Us,Them);


	calcCheckingSquares();
	assert(getSquareOfThePiece( x.getKingOfOtherPlayer() )!=squareNone);
	assert(getSquareOfThePiece( x.getKingOfActivePlayer() )!=squareNone);
	x.setHiddenCheckers( getHiddenCheckers<true>() );
	x.setPinnedPieces( getHiddenCheckers<false>() );

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif


}
/*! \brief do a move
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
void Position::doMove(const Move & m){
	assert( m );

	bool moveIsCheck = moveGivesCheck(m);

	insertState(getActualState());
	state &x = getActualState();

	x.setCurrentMove( m );




	tSquare from = m.getFrom();
	tSquare to = m.getTo();
	tSquare captureSquare = m.getTo();
	bitboardIndex piece = squares[from];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);

	bitboardIndex capture = ( m.isEnPassantMove() ? (x.isBlackTurn() ? whitePawns : blackPawns ) : squares[to] );
	assert(capture!=separationBitmap);
	assert(capture!=whitePieces);
	assert(capture!=blackPieces);


	// change side
	x.key ^= HashKeys::side;
	++ply;

	// update counter
	x.fiftyMoveCnt++;
	x.pliesFromNull++;

	// reset ep square
	if(x.epSquare!=squareNone)
	{
		assert(x.epSquare<squareNumber);
		x.key ^= HashKeys::ep[x.epSquare];
		x.epSquare = squareNone;
	}

	// do castle additional instruction
	if( m.isCastleMove() )
	{
		bool kingSide = m.isKingSideCastle();
		tSquare rFrom = kingSide? to+est: to+ovest+ovest;
		assert(rFrom<squareNumber);
		bitboardIndex rook = squares[rFrom];
		assert( isRook(rook) );
		tSquare rTo = kingSide? to+ovest: to+est;
		assert(rTo<squareNumber);
		movePiece(rook,rFrom,rTo);
		x.addMaterial( pstValue[rook][rTo] - pstValue[rook][rFrom] );

		x.key ^= HashKeys::keys[rFrom][rook];
		x.key ^= HashKeys::keys[rTo][rook];

	}

	// do capture
	if(capture)
	{
		if(isPawn(capture))
		{

			if( m.isEnPassantMove() )
			{
				captureSquare-=pawnPush( x.isBlackTurn() );
			}
			assert(captureSquare<squareNumber);
			x.pawnKey ^= HashKeys::keys[captureSquare][capture];
		}

		// remove piece
		removePiece(capture,captureSquare);
		// update material
		x.removeMaterial( pstValue[capture][captureSquare] );
		x.removeNonPawnMaterial( nonPawnValue[capture] );

		// update keys
		x.key ^= HashKeys::keys[captureSquare][capture];
		assert(getPieceCount(capture)<30);
		x.materialKey ^= HashKeys::keys[capture][getPieceCount(capture)]; // ->after removing the piece

		// reset fifty move counter
		x.fiftyMoveCnt = 0;
	}

	// update hashKey
	x.key ^= HashKeys::keys[from][piece] ^ HashKeys::keys[to][piece];
	movePiece(piece,from,to);

	x.addMaterial( pstValue[piece][to] - pstValue[piece][from] );

	// Update castle rights if needed
	if ( x.hasCastleRights() && (castleRightsMask[from] | castleRightsMask[to]))
	{
		eCastle cr = castleRightsMask[from] | castleRightsMask[to];
		assert((x.getCastleRights() & cr)<16);
		x.key ^= HashKeys::castlingRight[ x.getCastleRights() & cr ];
		x.clearCastleRight( cr );
	}



	if(isPawn(piece))
	{
		if(
				abs(from-to)==16
				&& (getAttackersTo((tSquare)((from+to)>>1))  & Them[Pawns])
		)
		{
			x.epSquare = (tSquare)((from+to)>>1);
			assert(x.epSquare<squareNumber);
			x.key ^= HashKeys::ep[x.epSquare];
		}
		if( m.isPromotionMove() )
		{
			bitboardIndex promotedPiece = getPieceOfPlayer( m.getPromotedPiece(), x.getNextTurn() );
			assert( isValidPiece(promotedPiece) );

			removePiece(piece,to);
			putPiece(promotedPiece,to);

			x.addMaterial( pstValue[promotedPiece][to] - pstValue[piece][to] );
			x.addNonPawnMaterial( nonPawnValue[promotedPiece] );


			x.key ^= HashKeys::keys[to][piece]^ HashKeys::keys[to][promotedPiece];
			x.pawnKey ^= HashKeys::keys[to][piece];
			x.materialKey ^= HashKeys::keys[promotedPiece][getPieceCount(promotedPiece)-1] ^ HashKeys::keys[piece][getPieceCount(piece)];
		}
		x.pawnKey ^= HashKeys::keys[from][piece] ^ HashKeys::keys[to][piece];
		x.fiftyMoveCnt = 0;
	}

	x.capturedPiece = capture;
	x.changeNextTurn();

	std::swap(Us,Them);


	x.setCheckers( 0 );
	if(moveIsCheck)
	{

		if( !m.isStandardMove() )
		{
			assert(getSquareOfThePiece( x.getKingOfActivePlayer() )<squareNumber);
			x.addCheckers( getAttackersTo(getSquareOfThePiece( x.getKingOfActivePlayer() ) ) & Them[Pieces] );
		}
		else
		{
			if(x.checkingSquares[piece] & bitSet(to)) // should be old state, but checkingSquares has not been changed so far
			{
				x.addCheckers( bitSet(to) );
			}
			if(x.thereAreHiddenCheckers() && (x.isHiddenChecker( from ) ) )	// should be old state, but hiddenCheckersCandidate has not been changed so far
			{
				if(!isRook(piece))
				{
					assert(getSquareOfThePiece( x.getKingOfActivePlayer() )<squareNumber);
					x.addCheckers( Movegen::attackFrom<whiteRooks>(getSquareOfThePiece( x.getKingOfActivePlayer() ),bitBoard[occupiedSquares]) & (Them[Queens] |Them[Rooks]) );
				}
				if(!isBishop(piece))
				{
					assert(getSquareOfThePiece( x.getKingOfActivePlayer() )<squareNumber);
					x.addCheckers( Movegen::attackFrom<whiteBishops>(getSquareOfThePiece( x.getKingOfActivePlayer() ),bitBoard[occupiedSquares]) & (Them[Queens] |Them[Bishops]) );
				}
			}
		}
	}

	calcCheckingSquares();
	assert(getSquareOfThePiece( x.getKingOfOtherPlayer() )<squareNumber);
	x.setHiddenCheckers( getHiddenCheckers<true>() );
	assert(getSquareOfThePiece(x.getKingOfActivePlayer() )<squareNumber);
	x.setPinnedPieces( getHiddenCheckers<false>() );

#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(1);
#endif


}

/*! \brief undo a move
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
void Position::undoMove()
{

	--ply;

	state x = getActualStateConst();
	const Move &m = x.getCurrentMove();
	assert( m );
	tSquare to = m.getTo();
	tSquare from = m.getFrom();
	bitboardIndex piece = squares[to];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);

	if( m.isPromotionMove() ){
		removePiece(piece,to);
		piece = (bitboardIndex)(piece > separationBitmap ? blackPawns : whitePawns);
		putPiece(piece,to);
	}

	if( m.isCastleMove() )
	{
		bool kingSide = m.isKingSideCastle();
		tSquare rFrom = kingSide? to+est: to+ovest+ovest;
		tSquare rTo = kingSide? to+ovest: to+est;
		assert(rFrom<squareNumber);
		assert(rTo<squareNumber);
		bitboardIndex rook = squares[rTo];
		assert( isRook(rook) );
		movePiece(rook,rTo,rFrom);

	}

	movePiece(piece,to,from);


	assert( isValidPiece(x.capturedPiece) || x.capturedPiece == empty);
	if(x.capturedPiece)
	{
		
		tSquare capSq = to;
		if( m.isEnPassantMove() )
		{
			capSq += pawnPush( x.isBlackTurn() );
		}
		assert(capSq<squareNumber);
		putPiece(x.capturedPiece,capSq);
	}
	removeState();

	std::swap(Us,Them);


#ifdef	ENABLE_CHECK_CONSISTENCY
	checkPosConsistency(0);
#endif

}

/*! \brief init the helper castle mash
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
void Position::initCastleRightsMask(void)
{
	for(int i=0;i<squareNumber;i++)
	{
		castleRightsMask[i] = eCastle(0);
	}
	castleRightsMask[E1] = wCastleOO | wCastleOOO;
	castleRightsMask[A1] = wCastleOOO;
	castleRightsMask[H1] = wCastleOO;
	castleRightsMask[E8] = bCastleOO | bCastleOOO;
	castleRightsMask[A8] = bCastleOOO;
	castleRightsMask[H8] = bCastleOO;
}


/*! \brief do a sanity check on the board
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
bool Position::checkPosConsistency(int nn) const
{
	return true;
	const state &x = getActualStateConst();
	if( x.getNextTurn() !=whiteTurn && x.getNextTurn() !=blackTurn)
	{
		sync_cout<<"nextMove error" <<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}

	// check board
	if(bitBoard[whitePieces] & bitBoard[blackPieces])
	{
		sync_cout<<"white piece & black piece intersected"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	if((bitBoard[whitePieces] | bitBoard[blackPieces]) !=bitBoard[occupiedSquares])
	{
		display();
		displayBitmap(bitBoard[whitePieces]);
		displayBitmap(bitBoard[blackPieces]);
		displayBitmap(bitBoard[occupiedSquares]);

		sync_cout<<"all piece problem"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	for(tSquare i=square0;i<squareNumber;i++)
	{
		bitboardIndex id=squares[i];

		if(id != empty && (bitBoard[id] & bitSet(i))==0)
		{
			sync_cout<<"board inconsistency"<<sync_endl;
			sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
			while(1){}
			return false;
		}
	}

	for (int i = whiteKing; i <= blackPawns; i++)
	{
		for (int j = whiteKing; j <= blackPawns; j++)
		{
			if(i!=j && i!= whitePieces && i!= separationBitmap && j!= whitePieces && j!= separationBitmap && (bitBoard[i] & bitBoard[j]))
			{
				sync_cout<<"bitboard intersection"<<sync_endl;
				sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
				while(1){}
				return false;
			}
		}
	}
	for (int i = whiteKing; i <= blackPawns; i++)
	{
		if(i!= whitePieces && i!= separationBitmap)
		{
			if(getPieceCount((bitboardIndex)i) != bitCnt(bitBoard[i]))
			{
				sync_cout<<"pieceCount Error"<<sync_endl;
				sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
				while(1){}
				return false;
			}
		}
	}


	bitMap test=0;
	for (int i = whiteKing; i < whitePieces; i++)
	{
		test |=bitBoard[i];
	}
	if(test!= bitBoard[whitePieces])
	{
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		sync_cout<<"white piece error"<<sync_endl;
		while(1){}
		return false;

	}
	test=0;
	for (int i = blackKing; i < blackPieces; i++)
	{
		test |=bitBoard[i];
	}
	if(test!= bitBoard[blackPieces])
	{
		sync_cout<<"black piece error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;

	}
	if(x.key != calcKey())
	{
		display();
		sync_cout<<"hashKey error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	if(x.pawnKey != calcPawnKey())
	{


		display();
		sync_cout<<"pawnKey error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	if(x.materialKey != calcMaterialKey())
	{
		sync_cout<<"materialKey error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}

	simdScore sc=calcMaterialValue();
	if((sc[0]!=x.getMaterialValue()[0]) || (sc[1]!=x.getMaterialValue()[1]))
	{
		display();
		sync_cout<<sc[0]<<":"<<x.getMaterialValue()[0]<<sync_endl;
		sync_cout<<sc[1]<<":"<<x.getMaterialValue()[1]<<sync_endl;
		sync_cout<<"material error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}
	simdScore score = calcNonPawnMaterialValue();
	if(score[0]!= x.getNonPawnValue()[0] ||
		score[1]!= x.getNonPawnValue()[1] ||
		score[2]!= x.getNonPawnValue()[2] ||
		score[3]!= x.getNonPawnValue()[3]
	){
		display();
		sync_cout<<score[0]<<":"<<x.getNonPawnValue()[0]<<sync_endl;
		sync_cout<<score[1]<<":"<<x.getNonPawnValue()[1]<<sync_endl;
		sync_cout<<score[2]<<":"<<x.getNonPawnValue()[2]<<sync_endl;
		sync_cout<<score[3]<<":"<<x.getNonPawnValue()[3]<<sync_endl;
		sync_cout<<"non pawn material error"<<sync_endl;
		sync_cout<<(nn?"DO error":"undoError") <<sync_endl;
		while(1){}
		return false;
	}





	return true;
}


/*! \brief calculate the perft result
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
unsigned long long Position::perft(unsigned int depth)
{

#define FAST_PERFT
#ifndef FAST_PERFT
	if (depth == 0) {
		return 1;
	}
#endif
	PerftTranspositionTable tt;
	
	unsigned long long tot = 0;
	if( perftUseHash && tt.retrieve(getKey(), depth, tot) )
	{
		return tot;
	}
	Movegen mg(*this);
#ifdef FAST_PERFT
	if(depth==1)
	{
		return mg.getNumberOfLegalMoves();
	}
#endif

	Move m;
	while ( ( m = mg.getNextMove() ) )
	{
		doMove(m);
		tot += perft(depth - 1);
		undoMove();
	}
	
	if( perftUseHash )
	{
		tt.store( getKey(), depth, tot );
	}
	
	return tot;

}

/*! \brief calculate the divide result
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
unsigned long long Position::divide(unsigned int depth)
{


	Movegen mg(*this);
	unsigned long long tot = 0;
	unsigned int mn=0;
	Move m;
	while ( ( m = mg.getNextMove() ) )
	{
		mn++;
		doMove(m);
		unsigned long long n= 1;
		if( depth>1)
		{
			n= perft(depth - 1);
		}
		else
		{
			n= 1;
		}
		tot += n;
		undoMove();
		sync_cout<<mn<<") "<<displayMove(*this,m)<<": "<<n<<sync_endl;

	}
	return tot;

}


/*! \brief calculate the checking squares given the king position
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
inline void Position::calcCheckingSquares(void)
{
	state &s = getActualState();
	bitboardIndex opponentKing = s.getKingOfOtherPlayer();
	assert( isKing(opponentKing));
	bitboardIndex attackingPieces = (bitboardIndex)(s.getNextTurn());
	//assert( isValidPiece(attackingPieces));


	tSquare kingSquare = getSquareOfThePiece(opponentKing);

	bitMap occupancy = bitBoard[occupiedSquares];

	s.checkingSquares[whiteKing+attackingPieces]=0;
	assert(kingSquare<squareNumber);
	assert( isValidPiece( (bitboardIndex)( whitePawns + attackingPieces ) ) );
	s.checkingSquares[whiteRooks+attackingPieces] = Movegen::attackFrom<whiteRooks>(kingSquare,occupancy);
	s.checkingSquares[whiteBishops+attackingPieces] = Movegen::attackFrom<whiteBishops>(kingSquare,occupancy);
	s.checkingSquares[whiteQueens+attackingPieces] = s.checkingSquares[whiteRooks+attackingPieces]|s.checkingSquares[whiteBishops+attackingPieces];
	s.checkingSquares[whiteKnights+attackingPieces] = Movegen::attackFrom<whiteKnights>(kingSquare);

	if(attackingPieces)
	{
		s.checkingSquares[whitePawns+attackingPieces] = Movegen::attackFrom<whitePawns>(kingSquare);
	}else
	{
		s.checkingSquares[whitePawns+attackingPieces] = Movegen::attackFrom<blackPawns>(kingSquare,1);
	}

	assert(blackPawns-attackingPieces>=0);
	s.checkingSquares[blackKing-attackingPieces] = 0;
	s.checkingSquares[blackRooks-attackingPieces] = 0;
	s.checkingSquares[blackBishops-attackingPieces] = 0;
	s.checkingSquares[blackQueens-attackingPieces] = 0;
	s.checkingSquares[blackKnights-attackingPieces] = 0;
	s.checkingSquares[blackPawns-attackingPieces]  =0;

}

/*! \brief get the hidden checkers/pinners of a position
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
template<bool our>
bitMap Position::getHiddenCheckers() const
{
	const state &x  = getActualStateConst();
	const tSquare kingSquare = getSquareOfThePiece( our ? x.getKingOfOtherPlayer(): x.getKingOfActivePlayer() );
	const eNextMove next = our ? x.getNextTurn(): x.getSwitchedTurn();
	
	assert(kingSquare<squareNumber);
	assert( isValidPiece( getPieceOfPlayer( Pawns, next ) ) );
	bitMap result = 0;
	bitMap pinners = Movegen::getBishopPseudoAttack(kingSquare) &(bitBoard[ getPieceOfPlayer( Bishops, next )]| bitBoard[getPieceOfPlayer( Queens, next) ] );
	pinners |= Movegen::getRookPseudoAttack(kingSquare) &(bitBoard[getPieceOfPlayer( Rooks, next )]| bitBoard[getPieceOfPlayer( Queens, next ) ] );

	while(pinners)
	{
		bitMap b = SQUARES_BETWEEN[kingSquare][iterateBit(pinners)] & bitBoard[occupiedSquares];
		if ( !moreThanOneBit(b) )
		{
			result |= b & bitBoard[ x.getPiecesOfActivePlayer() ];
		}
	}
	return result;

}

/*! \brief get all the attackers/defender of a given square with a given occupancy
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bitMap Position::getAttackersTo(const tSquare to, const bitMap occupancy) const
{
	assert(to<squareNumber);
	bitMap res = (Movegen::attackFrom<blackPawns>(to) & bitBoard[whitePawns])
			|(Movegen::attackFrom<whitePawns>(to) & bitBoard[blackPawns])
			|(Movegen::attackFrom<whiteKnights>(to) & (bitBoard[blackKnights]|bitBoard[whiteKnights]))
			|(Movegen::attackFrom<whiteKing>(to) & (bitBoard[blackKing]|bitBoard[whiteKing]));
	bitMap mask = (bitBoard[blackBishops]|bitBoard[whiteBishops]|bitBoard[blackQueens]|bitBoard[whiteQueens]);
	res |= Movegen::attackFrom<whiteBishops>(to,occupancy) & mask;
	mask = (bitBoard[blackRooks]|bitBoard[whiteRooks]|bitBoard[blackQueens]|bitBoard[whiteQueens]);
	res |=Movegen::attackFrom<whiteRooks>(to,occupancy) & mask;

	return res;
}


/*! \brief tell us if a move gives check before doing the move
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
bool Position::moveGivesCheck(const Move& m)const
{
	assert( m );
	tSquare from = m.getFrom();
	tSquare to = m.getTo();
	bitboardIndex piece = squares[from];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);
	const state &s = getActualStateConst();

	// Direct check ?
	if (s.checkingSquares[piece] & bitSet(to))
	{
		return true;
	}

	// Discovery check ?
	if(s.thereAreHiddenCheckers() && s.isHiddenChecker(from) )
	{
		// For pawn and king moves we need to verify also direction
		assert(getSquareOfThePiece( s.getKingOfOtherPlayer() )<squareNumber);
		if ( (!isPawn(piece)&& !isKing(piece)) || !squaresAligned(from, to, getSquareOfThePiece( s.getKingOfOtherPlayer() )))
			return true;
	}
	if( m.isStandardMove() )
	{
		return false;
	}

	tSquare kingSquare = getSquareOfThePiece( s.getKingOfOtherPlayer() );
	assert(kingSquare<squareNumber);

	if( m.isPromotionMove() )
	{
		assert( isValidPiece( getPieceOfPlayer( m.getPromotedPiece(), s.getNextTurn() ) ) );
		if( s.checkingSquares[ getPieceOfPlayer( m.getPromotedPiece(), s.getNextTurn() )] & bitSet(to))
		{
			return true;
		}
		bitMap occ= bitBoard[occupiedSquares] ^ bitSet(from);
		assert( isValidPiece( (bitboardIndex)(whiteQueens + m.getPromotionType() ) ) );
		switch(m.getPromotionType())
		{
			case Move::promQueen:
				return Movegen::attackFrom<whiteQueens>(to, occ) & bitSet(kingSquare);

			case Move::promRook:
				return Movegen::attackFrom<whiteRooks>(to, occ) & bitSet(kingSquare);

			case Move::promBishop:
				return Movegen::attackFrom<whiteBishops>(to, occ) & bitSet(kingSquare);

			case Move::promKnight:
				return Movegen::attackFrom<whiteKnights>(to, occ) & bitSet(kingSquare);



		}

	}
	else if( m.isCastleMove() )
	{
		tSquare kFrom = from;
		tSquare kTo = to;
		bool kingSide = m.isKingSideCastle();
		tSquare rFrom = kingSide ? to + est : to + ovest + ovest;
		tSquare rTo = kingSide ? to + ovest : to + est;
		assert(rFrom<squareNumber);
		assert(rTo<squareNumber);

		bitMap occ = (bitBoard[occupiedSquares] ^ bitSet(kFrom) ^ bitSet(rFrom)) | bitSet(rTo) | bitSet(kTo);
		return   (Movegen::getRookPseudoAttack(rTo) & bitSet(kingSquare))
			     && (Movegen::attackFrom<whiteRooks>(rTo,occ) & bitSet(kingSquare));
	}
	else if( m.isEnPassantMove() )
	{
		bitMap captureSquare = FILEMASK[m.getTo()] & RANKMASK[m.getFrom()];
		bitMap occ = bitBoard[occupiedSquares]^bitSet(m.getFrom())^bitSet(m.getTo())^captureSquare;
		return
				(Movegen::attackFrom<whiteRooks>(kingSquare, occ) & (Us[Queens] |Us[Rooks]))
			   | (Movegen::attackFrom<whiteBishops>(kingSquare, occ) & (Us[Queens] |Us[Bishops]));

	}

	return false;
}

bool Position::moveGivesDoubleCheck(const Move& m)const
{
	assert( m );
	tSquare from = m.getFrom();
	tSquare to = m.getTo();
	bitboardIndex piece = squares[from];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);
	const state &s = getActualStateConst();


	// Direct check ?
	return ((s.checkingSquares[piece] & bitSet(to)) && (s.thereAreHiddenCheckers() && s.isHiddenChecker( from ) ) );


}

bool Position::moveGivesSafeDoubleCheck(const Move& m)const
{
	assert( m );
	tSquare from = m.getFrom();
	tSquare to = m.getTo();
	bitboardIndex piece = squares[from];
	assert(piece!=occupiedSquares);
	assert(piece!=separationBitmap);
	assert(piece!=whitePieces);
	assert(piece!=blackPieces);
	const state& s = getActualStateConst();

	tSquare kingSquare = getSquareOfThePiece( s.getKingOfOtherPlayer() );
	return (!(Movegen::attackFrom<whiteKing>(kingSquare) & bitSet(to)) &&  (s.checkingSquares[piece] & bitSet(to)) && (s.thereAreHiddenCheckers() && s.isHiddenChecker(from)));


}


bool Position::isDraw(bool isPVline) const
{

	// Draw by material?

	const state & s = getActualStateConst();
	if (  !bitBoard[whitePawns] && !bitBoard[blackPawns]
		&&( ( (s.getNonPawnValue()[0]<= pieceValue[whiteBishops][0]) && s.getNonPawnValue()[2] == 0)
		|| ( (s.getNonPawnValue()[2]<= pieceValue[whiteBishops][0]) && s.getNonPawnValue()[0] == 0)
		)
	)
	{
		return true;
	}
	// Draw by the 50 moves rule?
	if ( s.fiftyMoveCnt > 99 )
	{
		if(!isInCheck())
		{
			return true;
		}

		Movegen mg(*this);
		if(mg.getNumberOfLegalMoves())
		{
			return true;
		}
	}

	// Draw by repetition?
	unsigned int counter=1;
	uint64_t actualkey = s.key;
	auto it = stateInfo.rbegin();
	

	int e = std::min( s.fiftyMoveCnt, s.pliesFromNull );
	if( e >= 4)
	{
		std::advance( it,2);
	}
	for(int i = 4 ;	i<=e;i+=2)
	{
		std::advance( it,2);
		if(it->key == actualkey)
		{
			counter++;
			if(!isPVline || counter>=3)
			{
				return true;
			}
		}
	}
	return false;
}

bool Position::isMoveLegal(const Move &m)const
{

	if( !m )
	{
		return false;
	}

	const state &s = getActualStateConst();
	const bitboardIndex piece = squares[m.getFrom()];
	assert( isValidPiece( piece ) || piece == empty );

	// pezzo inesistente
	if(piece == empty)
	{
		return false;
	}

	// pezzo del colore sbagliato
	if( s.isBlackTurn() ? !isblack(piece) : isblack(piece) )
	{
		return false;
	}

	//casa di destinazione irraggiungibile
	if(bitSet(m.getTo()) & Us[Pieces])
	{
		return false;
	}

	//scacco
	if( s.isInCheck() )
	{
		if( s.isInDoubleCheck() )  //scacco doppio posso solo muovere il re
		{
			if(!isKing(piece))
			{
				return false;
			}
		}
		else // scacco singolo i pezzi che non sono re possono catturare il pezzo attaccante oppure mettersi nel mezzo
		{

			if( !isKing(piece)
				&& !(
					((bitSet((tSquare)(m.getTo()-( m.isEnPassantMove() ? pawnPush( isBlackTurn() ) : 0)))) & s.getCheckers() )
					|| ((bitSet(m.getTo()) & SQUARES_BETWEEN[getSquareOfThePiece( s.getKingOfActivePlayer() ) ][ firstOne( s.getCheckers() ) ] ) )
				)
			)
			{
				return false;
			}
		}
	}
	if( (s.isPinned( m.getFrom() ) && !squaresAligned(m.getFrom(), m.getTo(), getSquareOfThePiece( s.getKingOfActivePlayer()  ) ) ) )
	{
		return false;
	}


	// promozione impossibile!!
	if(m.isPromotionMove() && ((RANKS[m.getFrom()]!=(s.isBlackTurn() ? 1 : 6 )) || !(isPawn(piece))))
	{
		return false;
	}

	// mossa mal formata
	//if( !m.isPromotionMove() && m.bit.promotion!=0)
	//{
	//	return false;
	//}
	//arrocco impossibile
	if( m.isCastleMove() )
	{
		if(!isKing(piece) || (FILES[m.getFrom()]!=FILES[E1]) || (abs(m.getFrom()-m.getTo())!=2 ) || (RANKS[m.getFrom()]!=RANKS[A1] && RANKS[m.getFrom()]!=RANKS[A8]))
		{
			return false;
		}
	}

	//en passant impossibile
	if( m.isEnPassantMove()  && (!isPawn(piece) || (m.getTo()) != s.epSquare))
	{
		return false;
	}

	//en passant impossibile
	if( !m.isEnPassantMove()  && isPawn(piece) && ( m.getTo() == s.epSquare))
	{
		return false;
	}




	switch(piece)
	{
		case whiteKing:
		case blackKing:
		{
			if( m.isCastleMove() )
			{
				Color color = s.isBlackTurn()? black : white;
				Position::eCastle cs = Position::state::calcCastleRight( m.isKingSideCastle() ? castleOO: castleOOO, color );
				Movegen mg(*this);
				if( !s.hasCastleRight( cs )
					|| !mg.isCastlePathFree( cs )
				)
				{
					return false;
				}
				if(m.getTo()>m.getFrom())
				{
					for(tSquare x=m.getFrom();x<=m.getTo() ;x++){
						if(getAttackersTo(x,bitBoard[occupiedSquares] & ~Us[King]) & Them[Pieces])
						{
							return false;
						}
					}
				}else{
					for(tSquare x=m.getTo();x<=m.getFrom() ;x++)
					{
						if(getAttackersTo(x,bitBoard[occupiedSquares] & ~Us[King]) & Them[Pieces])
						{
							return false;
						}
					}
				}
			}
			else{
				if(!(Movegen::attackFrom<whiteKing>((tSquare)m.getFrom()) &bitSet(m.getTo())) || (bitSet(m.getTo())&Us[Pieces]))
				{
					return false;
				}
				//king moves should not leave king in check
				if((getAttackersTo(m.getTo(),bitBoard[occupiedSquares] & ~Us[King]) & Them[Pieces]))
				{
					return false;
				}
			}





		}
			break;

		case whiteRooks:
		case blackRooks:
			assert(m.getFrom()<squareNumber);
			if(!(Movegen::getRookPseudoAttack(m.getFrom()) & bitSet(m.getTo())) || !(Movegen::attackFrom<whiteRooks>(m.getFrom(),bitBoard[occupiedSquares])& bitSet(m.getTo())))
			{
				return false;
			}
			break;

		case whiteQueens:
		case blackQueens:
			assert(m.getFrom()<squareNumber);
			if(
				!(
					(Movegen::getBishopPseudoAttack(m.getFrom()) | Movegen::getRookPseudoAttack(m.getFrom()))
					& bitSet(m.getTo())
				)
				||
				!(
					(

						Movegen::attackFrom<whiteBishops>(m.getFrom(),bitBoard[occupiedSquares])
						| Movegen::attackFrom<whiteRooks>(m.getFrom(),bitBoard[occupiedSquares])
					)
					& bitSet(m.getTo())
				)
			)
			{
				return false;
			}
			break;

		case whiteBishops:
		case blackBishops:
			if(!(Movegen::getBishopPseudoAttack(m.getFrom()) & bitSet(m.getTo())) || !(Movegen::attackFrom<whiteBishops>(m.getFrom(),bitBoard[occupiedSquares])& bitSet(m.getTo())))
			{
				return false;
			}
			break;

		case whiteKnights:
		case blackKnights:
			if(!(Movegen::attackFrom<whiteKnights>(m.getFrom())& bitSet(m.getTo())))
			{
				return false;
			}

			break;

		case whitePawns:

			if(
				// not valid pawn push
				(m.getFrom() + pawnPush(s.isBlackTurn())!= m.getTo() || (bitSet(m.getTo())&bitBoard[occupiedSquares]))
				// not valid pawn double push
				&& ((m.getFrom() + 2 * pawnPush(s.isBlackTurn())!= m.getTo()) || (RANKS[m.getFrom()]!=1) || ((bitSet(m.getTo()) | bitSet((tSquare)(m.getTo()-8)))&bitBoard[occupiedSquares]))
				// not valid pawn attack
				&& (!(Movegen::attackFrom<whitePawns>(m.getFrom())&bitSet(m.getTo())) || !((bitSet(m.getTo())) &(Them[Pieces]|bitSet(s.epSquare))))
			){
				return false;
			}
			if(RANKS[m.getFrom()]==6 && !m.isPromotionMove())
			{
				return false;

			}
			if( m.isEnPassantMove() ){

				bitMap captureSquare= FILEMASK[s.epSquare] & RANKMASK[m.getFrom()];
				bitMap occ= bitBoard[occupiedSquares]^bitSet(m.getFrom())^bitSet(s.epSquare)^captureSquare;
				tSquare kingSquare=getSquareOfThePiece( s.getKingOfActivePlayer() );
				assert(kingSquare<squareNumber);
				if((Movegen::attackFrom<whiteRooks>(kingSquare, occ) & (Them[Queens] | Them[Rooks]))|
							(Movegen::attackFrom<whiteBishops>(kingSquare, occ) & (Them[Queens] | Them[Bishops])))
				{
				return false;
				}
			}

			break;
		case blackPawns:
			if(
				// not valid pawn push
				(m.getFrom() + pawnPush(s.isBlackTurn())!= m.getTo() || (bitSet(m.getTo())&bitBoard[occupiedSquares]))
				// not valid pawn double push
				&& ((m.getFrom() + 2 * pawnPush(s.isBlackTurn())!= m.getTo()) || (RANKS[m.getFrom()]!=6) || ((bitSet(m.getTo()) | bitSet((tSquare)(m.getTo()+8)))&bitBoard[occupiedSquares]))
				// not valid pawn attack
				&& (!(Movegen::attackFrom<blackPawns>(m.getFrom())&bitSet(m.getTo())) || !((bitSet(m.getTo())) &(Them[Pieces]| bitSet(s.epSquare))))
			){
				return false;
			}

			if(RANKS[m.getFrom()]==1 && !

					m.isPromotionMove()){
				return false;

			}
			if( m.isEnPassantMove() ){
				bitMap captureSquare = FILEMASK[s.epSquare] & RANKMASK[m.getFrom()];
				bitMap occ = bitBoard[occupiedSquares]^bitSet(m.getFrom())^bitSet(s.epSquare)^captureSquare;
				tSquare kingSquare = getSquareOfThePiece( s.getKingOfActivePlayer() );
				assert(kingSquare<squareNumber);
				if((Movegen::attackFrom<whiteRooks>(kingSquare, occ) & (Them[Queens] | Them[Rooks]))|
							(Movegen::attackFrom<whiteBishops>(kingSquare, occ) & (Them[Queens] | Them[Bishops])))
				{
				return false;
				}
			}
			break;
		default:
			return false;

	}


	return true;
}


Position::Position():ply(0)
{
	stateInfo.clear();
	stateInfo.emplace_back(state());
	stateInfo[0].setNextTurn( whiteTurn );

	updateUsThem();

}


Position::Position(const Position& other):ply(other.ply), stateInfo(other.stateInfo), squares(other.squares), bitBoard(other.bitBoard)
{
	updateUsThem();
}


Position& Position::operator=(const Position& other)
{
	if (this == &other)
	{
		return *this;
	}

	ply = other.ply;

	stateInfo = other.stateInfo;
	squares = other.squares;
	bitBoard = other.bitBoard;

	updateUsThem();

	return *this;
}

inline void Position::updateUsThem()
{
	auto turn = getNextTurn();
	Us = &bitBoard[ turn ];
	Them = &bitBoard[ blackTurn - turn ];
}

/*! \brief put a piece on the board
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
inline void Position::putPiece(const bitboardIndex piece,const tSquare s)
{
	assert(s<squareNumber);
	assert( isValidPiece( piece ) );
	bitboardIndex color = piece > separationBitmap ? blackPieces : whitePieces;
	bitMap b=bitSet(s);

	assert(squares[s]==empty);

	squares[s] = piece;
	bitBoard[piece] |= b;
	bitBoard[occupiedSquares] |= b;
	bitBoard[color] |= b;
}

/*! \brief move a piece on the board
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
inline void Position::movePiece(const bitboardIndex piece,const tSquare from,const tSquare to)
{
	assert(from<squareNumber);
	assert(to<squareNumber);
	assert( isValidPiece( piece ) );
	// index[from] is not updated and becomes stale. This works as long
	// as index[] is accessed just by known occupied squares.
	assert(squares[from]!=empty);
	assert(squares[to]==empty);
	bitMap fromTo = bitSet(from) ^ bitSet(to);
	bitboardIndex color = piece > separationBitmap ? blackPieces : whitePieces;
	bitBoard[occupiedSquares] ^= fromTo;
	bitBoard[piece] ^= fromTo;
	bitBoard[color] ^= fromTo;
	squares[from] = empty;
	squares[to] = piece;


}
/*! \brief remove a piece from the board
	\author STOCKFISH
	\version 1.0
	\date 27/10/2013
*/
inline void Position::removePiece(const bitboardIndex piece,const tSquare s)
{

	assert(!isKing(piece));
	assert(s<squareNumber);
	assert( isValidPiece( piece) );
	assert(squares[s]!=empty);

	// WARNING: This is not a reversible operation. If we remove a piece in
	// do_move() and then replace it in undo_move() we will put it at the end of
	// the list and not in its original place, it means index[] and pieceList[]
	// are not guaranteed to be invariant to a do_move() + undo_move() sequence.
	bitboardIndex color = piece  > separationBitmap ? blackPieces : whitePieces;
	bitMap b = bitSet(s);
	bitBoard[occupiedSquares] ^= b;
	bitBoard[piece] ^= b;
	bitBoard[color] ^= b;

	squares[s] = empty;

}
