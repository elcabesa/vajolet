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

#include <cstdint>

#include "position.h"
#include "bitops.h"
#include "movegen.h"
#include "vajolet.h"

  
 
//---------------------------------------------
//	MATERIAL KEYS
//---------------------------------------------




std::unordered_map<uint64_t, Position::materialStruct> Position::materialKeyMap;


bool Position::evalKxvsK(Score& res)
{
	//display();
	Color StrongColor = bitCnt(getBitmap(whitePieces))>1  ? white : black;
	tSquare winKingSquare;
	tSquare losKingSquare;
	bitboardIndex pieces;
	int mul = 1;
	if(StrongColor == white)
	{
		winKingSquare = getSquareOfThePiece(whiteKing);
		losKingSquare = getSquareOfThePiece(blackKing);
		pieces = whitePieces;

		mul = 1;
	}
	else
	{
		winKingSquare = getSquareOfThePiece(blackKing);
		losKingSquare = getSquareOfThePiece(whiteKing);
		pieces = blackPieces;

		mul = -1;
	}
	Movegen mg(*this);
	if( mg.getNumberOfLegalMoves() == 0 )
	{
		res = 0;
		return true;
	}


	res = SCORE_KNOWN_WIN + 50000;
	res -= 10 * SQUARE_DISTANCE[winKingSquare][losKingSquare];// devo tenere il re vicino
	res += 20 * SQUARE_DISTANCE[losKingSquare][E4];// devo portare il re avversario vicino al bordo
	res += 50 * bitCnt(getBitmap(pieces));
	assert( res < SCORE_MATE_IN_MAX_PLY);

	res *= mul;
	return true;

}

bool Position::evalKNPvsK(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	if(Pcolor == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		int pawnFile = FILES[pawnSquare];
		int pawnRank = RANKS[pawnSquare];
		if( (pawnFile ==0 || pawnFile ==7) && pawnRank == 6 )
		{

			res = 0;
			return true;

		}
	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		int pawnFile = FILES[pawnSquare];
		int pawnRank = RANKS[pawnSquare];
		if( (pawnFile ==0 || pawnFile ==7) && pawnRank == 1 )
		{
			res = 10;
			return true;
		}
	}
	return false;

}


bool Position::evalKBPsvsK(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white : black;
	bitMap pawns;
	tSquare bishopSquare;
	
	bitMap HFile = FILEMASK[H1];
	bitMap AFile = FILEMASK[A1];
	
	if(Pcolor == white)
	{	
		pawns = getBitmap(whitePawns);
		int pawnFile = FILES[firstOne( pawns )];
		
		if(  0 == ( pawns & ~AFile ) || 0 == ( pawns & ~HFile ) )
		{
			bishopSquare = getSquareOfThePiece(whiteBishops);
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][7]] != SQUARE_COLOR[bishopSquare])
			{
				tSquare kingSquare = getSquareOfThePiece(blackKing);
				if(RANKS[kingSquare] >= 6  && abs( pawnFile - FILES[kingSquare] ) <= 1 )
				{
					res = 0;
					return true;
				}
			}
		}
	}
	else
	{
		pawns = getBitmap(blackPawns);
		int pawnFile = FILES[firstOne( pawns )];
		if( 0 == ( pawns & ~AFile ) || 0 == ( pawns & ~HFile ) )
		{
			bishopSquare = getSquareOfThePiece(blackBishops);
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][0]] != SQUARE_COLOR[ bishopSquare ])
			{
				tSquare kingSquare = getSquareOfThePiece(whiteKing);
				if(RANKS[kingSquare] <= 1  && abs(pawnFile - FILES[kingSquare]) <= 1)
				{
					res = 0;
					return true;
				}
			}
		}
	}
	return false;

}

bool Position::evalKQvsKP(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white  :black;
	tSquare pawnSquare;
	tSquare winningKingSquare;
	tSquare losingKingSquare;


	if(Pcolor == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		winningKingSquare = getSquareOfThePiece(blackKing);
		losingKingSquare = getSquareOfThePiece(whiteKing);

		int pawnFile = FILES[pawnSquare];
		int pawnRank = RANKS[pawnSquare];
		res = -100 * ( 7 - SQUARE_DISTANCE[winningKingSquare][losingKingSquare] );

		if(
				pawnRank != 6
				|| SQUARE_DISTANCE[losingKingSquare][pawnSquare] != 1
				|| (pawnFile == 1 || pawnFile == 3 || pawnFile == 4 || pawnFile == 6) )
		{
			res -= 90000;
		}
		return true;

	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		winningKingSquare = getSquareOfThePiece(whiteKing);
		losingKingSquare = getSquareOfThePiece(blackKing);

		int pawnFile = FILES[pawnSquare];
		int pawnRank = RANKS[pawnSquare];
		res = 100 * ( 7 - SQUARE_DISTANCE[winningKingSquare][losingKingSquare] );

		if(
				pawnRank != 1
				|| SQUARE_DISTANCE[losingKingSquare][pawnSquare] != 1
				|| (pawnFile == 1 || pawnFile == 3 || pawnFile == 4 || pawnFile == 6) )
		{
			res += 90000;
		}
		return true;

	}
	return false;

}


bool Position::evalKRPvsKr(Score& res)
{
	Color Pcolor = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	if( Pcolor == white )
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		if(	FILES[pawnSquare] == FILES[getSquareOfThePiece(blackKing)]
		    && RANKS[pawnSquare] <= 6
		    && RANKS[pawnSquare] < RANKS[getSquareOfThePiece(blackKing)]
		)
		{
			res = 128;
			return true;
		}
	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		if(	FILES[pawnSquare] == FILES[getSquareOfThePiece(whiteKing)]
			&& RANKS[pawnSquare] >= 1
			&& RANKS[pawnSquare] > RANKS[getSquareOfThePiece(whiteKing)]
		)
		{
			res=128;
			return true;
		}

	}
	return false;

}

bool Position::evalKBNvsK( Score& res)
{
	Color color = getBitmap(whiteBishops) ? white : black;
	tSquare bishopSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	tSquare mateSquare1, mateSquare2;
	int mul = 1;
	if(color == white)
	{
		mul = 1;
		bishopSquare = getSquareOfThePiece(whiteBishops);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		mul = -1;
		bishopSquare = getSquareOfThePiece(blackBishops);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	int mateColor = SQUARE_COLOR[bishopSquare];
	if(mateColor == 0)
	{
		mateSquare1 = A1;
		mateSquare2 = H8;
	}
	else
	{
		mateSquare1 = A8;
		mateSquare2 = H1;
	}

	res = SCORE_KNOWN_WIN + 20000;

	res -= 5 * SQUARE_DISTANCE[enemySquare][kingSquare];// devo tenere il re vicino
	res -= 10 * std::min( SQUARE_DISTANCE[enemySquare][mateSquare1], SQUARE_DISTANCE[enemySquare][mateSquare2]);// devo portare il re avversario nel giusto angolo

	res *=mul;
	return true;

}

bool Position::evalKQvsK(Score& res)
{
	Color color = getBitmap(whiteQueens) ? white : black;
	tSquare kingSquare;
	tSquare enemySquare;
	
	Movegen mg(*this);
	if( mg.getNumberOfLegalMoves() == 0 )
	{
		res = 0;
		return true;
	}

	int mul = 1;
	if(color == white)
	{
		mul = 1;
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		mul = -1;
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	res = SCORE_KNOWN_WIN + 40000;
	res -= 10 * SQUARE_DISTANCE[enemySquare][kingSquare];// devo tenere il re vicino
	res += 20 * SQUARE_DISTANCE[enemySquare][E4];// devo portare il re avversario vicino al bordo

	res *= mul;
	return true;

}

bool Position::evalKRvsK(Score& res)
{
	Color color = getBitmap(whiteRooks) ? white : black;
	tSquare kingSquare;
	tSquare enemySquare;

	Movegen mg(*this);
	if( mg.getNumberOfLegalMoves() == 0 )
	{
		res = 0;
		return true;
	}
	
	int mul = 1;
	if(color == white)
	{
		mul = 1;
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		mul = -1;
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	res = SCORE_KNOWN_WIN + 30000;
	res -= 10 * SQUARE_DISTANCE[enemySquare][kingSquare];// devo tenere il re vicino
	res += 20 * SQUARE_DISTANCE[enemySquare][E4];// devo portare il re avversario vicino al bordo

	res *= mul;
	return true;

}

bool Position::kingsDirectOpposition()
{
	if(
			(getSquareOfThePiece(whiteKing) + 16 == getSquareOfThePiece(blackKing) )
			/*||
			(getSquareOfThePiece(whiteKing) == getSquareOfThePiece(blackKing) +16 )*/
	)
	{
		return true;
	}
	return false;

}

bool Position::evalKPvsK(Score& res)
{
	Color color = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	tSquare kingSquare;
	tSquare enemySquare;

	if(color == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);


		tSquare promotionSquare = BOARDINDEX[ FILES[pawnSquare] ][7];
		const int relativeRank = RANKS[pawnSquare];
		// Rule of the square
		if ( std::min( 5, (int)(7- relativeRank)) <  std::max(SQUARE_DISTANCE[enemySquare][promotionSquare] - (getNextTurn() == whiteTurn? 0 : 1) , 0) )
		{
			res = SCORE_KNOWN_WIN + relativeRank;
			return true;
		}
		if(FILES[pawnSquare] !=0 && FILES[pawnSquare] != 7)
		{

			if(SQUARE_DISTANCE[enemySquare][pawnSquare] >= 2 || getNextTurn() == Position::whiteTurn )
			{
				//winning king on a key square
				if(relativeRank < 4)
				{
					if(kingSquare >= pawnSquare + 15 && kingSquare <= pawnSquare + 17 )
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}
				}
				else if(relativeRank < 6)
				{
					if((kingSquare >= pawnSquare + 15 && kingSquare <= pawnSquare + 17) || (kingSquare >= pawnSquare + 7 && kingSquare <= pawnSquare + 9))
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}
				}
				else{

					if((kingSquare >= pawnSquare - 1 && kingSquare <= pawnSquare + 1 ) || (kingSquare >= pawnSquare + 7 && kingSquare <= pawnSquare + 9))
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}

				}

				// 3 rules for winning, if  conditions are met -> it's won
				unsigned int count = 0;
				if(kingSquare == pawnSquare + 8) count++;
				if(getNextTurn() == blackTurn && kingsDirectOpposition()) count++;
				if(RANKS[kingSquare] == 5) count++;

				if(count > 1)
				{
					res = SCORE_KNOWN_WIN + relativeRank;
					return true;
				}

			}
			//draw rule
			if((enemySquare==pawnSquare+8) || (enemySquare==pawnSquare+16 && RANKS[enemySquare]!=7))
			{
				res = 0;
				return true;
			}
		}
		else
		{
			//ROOKS PAWN
			if(abs(FILES[enemySquare] - FILES[pawnSquare]) <= 1  && RANKS[enemySquare] > 5 )
			{
				res = 0;
				return true;
			}


		}
	}
	else{
		pawnSquare = getSquareOfThePiece(blackPawns);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);



		tSquare promotionSquare = BOARDINDEX[FILES[pawnSquare]][0];
		const int relativeRank = 7 - RANKS[pawnSquare];
		// Rule of the square
		if ( std::min( 5, (int)( 7 - relativeRank)) <  std::max(SQUARE_DISTANCE[enemySquare][promotionSquare] - (getNextTurn() == blackTurn ? 0 : 1 ), 0) )
		{
			res = -SCORE_KNOWN_WIN - relativeRank;
			return true;
		}
		if(FILES[pawnSquare] != 0 && FILES[pawnSquare] != 7)
		{
			if(SQUARE_DISTANCE[enemySquare][pawnSquare] >= 2 || getNextTurn() == blackTurn)
			{
				//winning king on a key square
				if(relativeRank < 4)
				{
					if(kingSquare >= pawnSquare - 17 && kingSquare <= pawnSquare - 15 )
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				else if(relativeRank < 6)
				{
					if((kingSquare >= pawnSquare - 17 && kingSquare <= pawnSquare - 15) || (kingSquare >= pawnSquare - 9 && kingSquare <= pawnSquare - 7 ) )
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				else{

					if((kingSquare >= pawnSquare - 1 && kingSquare <= pawnSquare + 1) || (kingSquare >= pawnSquare - 9 && kingSquare <= pawnSquare - 7))
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				// 3 rules for winning, if  conditions are met -> it's won
				unsigned int count = 0;
				if(kingSquare == pawnSquare - 8) count++;
				if(getNextTurn() == whiteTurn && kingsDirectOpposition()) count++;
				if(RANKS[kingSquare] == 2) count++;

				if(count > 1)
				{
					res = -SCORE_KNOWN_WIN - relativeRank;
					return true;
				}
			}
			//draw rule
			if((enemySquare == pawnSquare - 8) || (enemySquare == pawnSquare - 16 && RANKS[enemySquare] != 0) )
			{
				res = 0;
				return true;
			}
		}
		else
		{
			//ROOKS PAWN
			if(abs(FILES[enemySquare] - FILES[pawnSquare]) <= 1  && RANKS[enemySquare] < 2)
			{
				res = 0;
				return true;
			}


		}
	}


	return false;
}

bool Position::evalKPsvsK(Score& res)
{
	Color color = getBitmap(whitePawns) ? white : black;
	
	tSquare kingSquare;
	bitMap pawns;
	
	bitMap HFile = FILEMASK[H1];
	bitMap AFile = FILEMASK[A1];
	
	// If all pawns are ahead of the king, on a single rook file and
	// the king is within one file of the pawns, it's a draw.
	if( color == white )
	{
		kingSquare = getSquareOfThePiece( blackKing );
		pawns = getBitmap(whitePawns);
	}
	else
	{
		kingSquare = getSquareOfThePiece( whiteKing );
		pawns = getBitmap(blackPawns);
	}
	
	if( 
			// re e pedoni sono al massimo su colonne adiacenti
			(std::abs(FILES[ firstOne( pawns ) ] - FILES[ kingSquare ])<=1 )
			// se tutti i pedoni sono sulla colonna A oppure tutti i pedoni sono sulla colonna H
			&&(  0 ==( pawns & ~AFile ) || 0 == ( pawns & ~HFile ) )
			// se tutti i pedoni sono davanti al re
			&& ( ( ~PASSED_PAWN[ 1 - color ][kingSquare] & pawns) == 0 )
	)
	{
		res = 0;
		return true;
	}
	return false;
}


bool Position::evalOppositeBishopEndgame(Score& res)
{
	if(SQUARE_COLOR[getSquareOfThePiece(blackBishops)] != SQUARE_COLOR[ getSquareOfThePiece(whiteBishops)])
	{
		unsigned int pawnCount = 0;
		int pawnDifference = 0;
		unsigned int freePawn = 0;

		bitMap pawns= getBitmap(whitePawns);
		while(pawns)
		{
			pawnCount++;
			pawnDifference++;
			tSquare pawn = iterateBit(pawns);
			if( !(PASSED_PAWN[0][pawn] & getBitmap(blackPawns)))
			{
				if(!(Movegen::getBishopPseudoAttack(getSquareOfThePiece(blackBishops)) & SQUARES_IN_FRONT_OF[0][pawn] ))
				{
					res = 256;
					freePawn++;
				}
			}
		}

		pawns= getBitmap(blackPawns);
		while(pawns)
		{
			pawnCount++;
			pawnDifference--;

			tSquare pawn = iterateBit(pawns);
			if(!( PASSED_PAWN[1][pawn] & getBitmap(whitePawns)))
			{
				if(!(Movegen::getBishopPseudoAttack(getSquareOfThePiece(whiteBishops)) & SQUARES_IN_FRONT_OF[1][pawn] ))
				{
					freePawn++;
					res = 256;
				}
			}
		}

		if(freePawn == 0 && std::abs(pawnDifference) <= 1 )
		{
			res = std::min(20 + pawnCount * 25, (unsigned int)256);
			return true;
		}

	}
	return false;

}

bool Position::evalKRvsKm(Score& res)
{
	res = 64;
	return true;
}

bool Position::evalKNNvsK(Score& res)
{
	res = 10;
	return true;
}


void Position::initMaterialKeys(void)
{
	/*---------------------------------------------
	 *
	 * K vs K		->	draw
	 * km vs k		->	draw
	 * km vs km		->	draw
	 * knn vs k		->	draw
	 * kmm vs km	->	draw
	 *
	 * kbp vs k		->	probable draw on the rook file
	 *
	 * kb vs kpawns	-> bishop cannot win
	 * kn vs kpawns	-> bishop cannot win
	 *
	 * kbn vs k		-> win
	 * opposite bishop endgame -> look drawish
	 * kr vs km		-> look drawish
	 * knn vs k		-> very drawish
	 * kq vs kp
	 ----------------------------------------------*/


	Position p;
	uint64_t key;

	static const struct{
		std::string fen;
		materialStruct::tType type;
		bool (Position::*pointer)(Score &);
		Score val;
	} Endgames[] = {
			// DRAWN
			{"k7/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },		//k vs k

			{"kb6/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },	//kb vs k
			{"k7/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },	//kn vs k
			{"k7/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },	//kn vs kn
			{"kn6/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },	//kn vs kb
			{"kb6/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },	//kb vs kb

			{"knn5/8/8/8/8/8/8/7K w - -",materialStruct::exact, nullptr, 0 },	//knn vs k
			{"k7/8/8/8/8/8/8/5NNK w - -",materialStruct::exact, nullptr, 0 },

			{"knn5/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"knn5/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },
			{"kbn5/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kbn5/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },
			{"kbb5/8/8/8/8/8/8/6NK w - -",materialStruct::exact, nullptr, 0 },
			{"kbb5/8/8/8/8/8/8/6BK w - -",materialStruct::exact, nullptr, 0 },

			{"knp5/8/8/8/8/8/8/7K w - -",materialStruct::multiplicativeFunction, &Position::evalKNPvsK, 0 },
			{"k7/8/8/8/8/8/8/5KNP w - -",materialStruct::multiplicativeFunction, &Position::evalKNPvsK, 0 },

			{"kn6/8/8/8/8/8/8/5NNK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5NNK w - -",materialStruct::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/5BNK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5BNK w - -",materialStruct::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/5BBK w - -",materialStruct::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5BBK w - -",materialStruct::exact, nullptr, 0 },

			{"kb6/8/8/8/8/8/7P/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/6PP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/5PPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/4PPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/3PPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/2PPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/1PPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/PPPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },

			{"k7/7p/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/6pp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/5ppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/4pppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/3ppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/2pppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/1ppppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/pppppppp/8/8/8/8/8/6BK w - -",materialStruct::saturationH, nullptr, 0 },

			{"kn6/8/8/8/8/8/7P/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/6PP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/5PPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/4PPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/3PPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/2PPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/1PPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/PPPPPPPP/7K w - -",materialStruct::saturationL, nullptr, 0 },

			{"k7/7p/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/6pp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/5ppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/4pppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/3ppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/2pppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/1ppppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },
			{"k7/pppppppp/8/8/8/8/8/6NK w - -",materialStruct::saturationH, nullptr, 0 },

			{"k7/8/8/8/8/8/8/5BPK w - -",materialStruct::exactFunction, &Position::evalKBPsvsK, 0 },
			{"k7/8/8/8/8/8/8/4BPPK w - -",materialStruct::exactFunction, &Position::evalKBPsvsK, 0 },
			{"k7/8/8/8/8/8/8/3BPPPK w - -",materialStruct::exactFunction, &Position::evalKBPsvsK, 0 },
			{"kbp5/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKBPsvsK, 0 },
			{"kbpp4/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKBPsvsK, 0 },
			{"kbppp3/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKBPsvsK, 0 },
			
			{"k7/8/8/8/8/8/8/5BNK w - -",materialStruct::exactFunction, &Position::evalKBNvsK, 0 },
			{"kbn5/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKBNvsK, 0 },

			{"k7/8/8/8/8/8/8/6QK w - -",materialStruct::exactFunction, &Position::evalKQvsK, 0 },
			{"kq6/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKQvsK, 0 },

			{"k7/8/8/8/8/8/8/6RK w - -",materialStruct::exactFunction, &Position::evalKRvsK, 0 },
			{"kr6/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKRvsK, 0 },

			{"k7/8/8/8/8/8/8/6PK w - -",materialStruct::exactFunction, &Position::evalKPvsK, 0 },
			{"kp6/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPvsK, 0 },
			
			{"k7/8/8/8/8/8/8/5PPK w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/4PPPK w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/3PPPPK w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/2PPPPPK w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/1PPPPPPK w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/PPPPPPPK w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"kpp5/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"kppp4/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"kpppp3/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"kppppp2/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"kpppppp1/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			{"kpppppp1/8/8/8/8/8/8/7K w - -",materialStruct::exactFunction, &Position::evalKPsvsK, 0 },
			
			
			
			

			{"kr6/8/8/8/8/8/8/6NK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"kr6/8/8/8/8/8/8/6BK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"kb6/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"kn6/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },

			{"knn5/8/8/8/8/8/8/7K w - -",materialStruct::multiplicativeFunction, &Position::evalKNNvsK, 0 },
			{"k7/8/8/8/8/8/8/5NNK w - -",materialStruct::multiplicativeFunction, &Position::evalKNNvsK, 0 },

			{"kr6/8/8/8/8/8/8/5PRK w - -",materialStruct::multiplicativeFunction, &Position::evalKRPvsKr, 0 },
			{"krp5/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRPvsKr, 0 },
			
			{"kr6/8/8/8/8/8/8/5NRK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"krn5/8/8/8/8/8/8/6RK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			
			{"krn5/8/8/8/8/8/8/4NNRK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },
			{"krnn4/8/8/8/8/8/8/5NRK w - -",materialStruct::multiplicativeFunction, &Position::evalKRvsKm, 0 },

			{"kq6/8/8/8/8/8/8/6PK w - -",materialStruct::exactFunction, &Position::evalKQvsKP, 0 },
			{"kp6/8/8/8/8/8/8/6QK w - -",materialStruct::exactFunction, &Position::evalKQvsKP, 0 }

	};

	materialStruct t;

	for (auto& eg : Endgames)
	{
		p.setupFromFen(eg.fen);
		key = p.getMaterialKey();
		t.type = eg.type;
		t.pointer = eg.pointer;
		t.val = eg.val;
		materialKeyMap.insert({key,t});
	}

	//------------------------------------------
	//	opposite bishop endgame
	//------------------------------------------
	for(int wp=0;wp<=8;wp++){
		for(int bp=0;bp<=8;bp++){
			if(wp!=0 || bp !=0){
				std::string s="kb6/";
				for(int i=0;i<bp;i++){
					s+="p";
				}
				if(bp!=8){s+=std::to_string(8-bp);}
				s+="/8/8/8/8/";
				for(int i=0;i<wp;i++){
					s+="P";
				}
				if(wp!=8){s+=std::to_string(8-wp);}
				s+="/6BK w - -";
				p.setupFromFen(s);
				key = p.getMaterialKey();
				t.type=materialStruct::multiplicativeFunction;
				t.pointer=&Position::evalOppositeBishopEndgame;
				t.val=0;
				materialKeyMap.insert({key,t});
			}
		}
	}
}
