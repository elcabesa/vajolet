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

#include "position.h"
#include "vajolet.h"

  
 
//---------------------------------------------
//	MATERIAL KEYS
//---------------------------------------------
std::unordered_map<tKey, Position::materialStruct> Position::_materialKeyMap;

/**********************************************
eval king and pieces vs lone king
**********************************************/
bool Position::_evalKxvsK(Score& res) const
{
	const Color StrongColor = bitCnt(getBitmap(whitePieces))>1  ? white : black;
	tSquare winKingSquare;
	tSquare losKingSquare;
	bitboardIndex pieces;
	int mul;
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
	
	if( getNumberOfLegalMoves() == 0 )
	{
		res = 0;
		return true;
	}


	res = SCORE_KNOWN_WIN + 50000;
	res -= 10 * distance(winKingSquare,losKingSquare);// devo tenere il re vicino
	res += 20 * centerDistance(losKingSquare);// devo portare il re avversario vicino al bordo
	res += 50 * bitCnt(getBitmap(pieces));
	assert( res < SCORE_MATE_IN_MAX_PLY);

	res *= mul;
	return true;

}

/**********************************************
eval king Knight and pawn vs lone king, 
it looks drawish if the pawn is on seventh and on the edge of the board
**********************************************/
bool Position::_evalKNPvsK(Score& res) const
{
	const Color Pcolor = getBitmap(whitePawns) ? white : black;
	const tSquare pawnSquare = getSquareOfThePiece( Pcolor ? blackPawns: whitePawns);
	const tRank relativeRank = getRelativeRankOf( pawnSquare, Pcolor);	
	const tFile pawnFile = getFileOf(pawnSquare);
	
	if( isLateralFile( pawnFile ) && relativeRank == RANK7 )
	{
		res = 0;
		return true;
	}
	return false;

}

/**********************************************
eval king Bishop and pawns vs lone king, 
it looks drawish if all the pawns are on the edge of the board and the bishop is of the wrong color
**********************************************/
bool Position::_evalKBPsvsK(Score& res) const
{
	const Color Pcolor = getBitmap(whitePawns) ? white : black;
	bitMap pawns;
	tSquare bishopSquare;
	tSquare kingSquare;
	
	bitMap HFile = fileMask(H1);
	bitMap AFile = fileMask(A1);
	
	if(Pcolor == white)
	{	
		pawns = getBitmap(whitePawns);
		bishopSquare = getSquareOfThePiece(whiteBishops);
		kingSquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		pawns = getBitmap(blackPawns);
		bishopSquare = getSquareOfThePiece(blackBishops);
		kingSquare = getSquareOfThePiece(whiteKing);
	}
	
	tFile pawnFile = getFileOf( firstOne( pawns ) );
	// all the pawn are on the A file or on the H file
	if(  
		// all the pawn are on the A file or on the H file
		(0 == ( pawns & ~AFile ) || 0 == ( pawns & ~HFile ) )
		// the square of promotion is not protected by bishop
		&& ( getSquareColor( getSquare( pawnFile, getRelativeRankOf(  A8, Pcolor ) ) ) != getSquareColor(bishopSquare) )
		// the defending king is near the promotion square
		&& ( getRelativeRankOf(kingSquare, Pcolor) >= RANK7 && abs( pawnFile - getFileOf(kingSquare) ) <= 1 )
	)
	{
		res = 0;
		return true;
	}	
	return false;

}

/**********************************************
eval king and queen vs king and pawn, 
it looks drawish if the promoting pawn is on column A or C
**********************************************/
bool Position::_evalKQvsKP(Score& res) const
{
	Color pColor = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	tSquare winningKingSquare;
	tSquare losingKingSquare;
	int mul;

	if(pColor == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		winningKingSquare = getSquareOfThePiece(blackKing);
		losingKingSquare = getSquareOfThePiece(whiteKing);
		mul = 1;
	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		winningKingSquare = getSquareOfThePiece(whiteKing);
		losingKingSquare = getSquareOfThePiece(blackKing);
		mul = -1;
	}
	
	int pawnFile = getFileOf(pawnSquare);
	int pawnRank = getRelativeRankOf(pawnSquare, pColor);
	res = -100 * ( 7 - distance(winningKingSquare,losingKingSquare) );
	
	if(
			pawnRank != RANK7
			|| distance(losingKingSquare,pawnSquare) != 1
			|| (pawnFile == FILEB || pawnFile == FILED || pawnFile == FILEE || pawnFile == FILEG) )
	{
		res -= 90000;
	}
	res *= mul;
	return true;

}

/**********************************************
eval king, rook and pawn vs king and rook, 
help handling lucena and philidor positions
**********************************************/
bool Position::_evalKRPvsKr(Score& res) const
{
	Color Pcolor = getBitmap(whitePawns) ? white : black;
	bitboardIndex pawnPiece;
	bitboardIndex enemyKing;
	
	if( Pcolor == white )
	{
		pawnPiece = whitePawns;
		enemyKing = blackKing;
	}
	else
	{
		pawnPiece = blackPawns;
		enemyKing = whiteKing;
	}
	
	tSquare pawnSquare = getSquareOfThePiece(pawnPiece);
	if(	getFileOf(pawnSquare) == getFileOf(getSquareOfThePiece(enemyKing))
		&& getRelativeRankOf(pawnSquare, Pcolor) <= RANK7
		&& getRelativeRankOf(pawnSquare, Pcolor) < getRelativeRankOf(getSquareOfThePiece(enemyKing), Pcolor)
	)
	{
		res = 128;
		return true;
	}
	return false;

}

/**********************************************
eval king, bishop and knight vs lone king, 
the rook shall be pyushed toward thre right corner and the winning king shall help the pieces
**********************************************/
bool Position::_evalKBNvsK( Score& res) const
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

	int mateColor = getSquareColor(bishopSquare);
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

	res -= 5 * distance(enemySquare,kingSquare);// devo tenere il re vicino
	res -= 10 * std::min( distance(enemySquare,mateSquare1), distance(enemySquare,mateSquare2));// devo portare il re avversario nel giusto angolo

	res *=mul;
	return true;

}

bool Position::_evalKQvsK(Score& res) const
{
	Color color = getBitmap(whiteQueens) ? white : black;
	tSquare kingSquare;
	tSquare enemySquare;
	
	if( getNumberOfLegalMoves() == 0 )
	{
		res = 0;
		return true;
	}

	int sign = 1;
	if(color == white)
	{
		sign = 1;
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
	}
	else
	{
		sign = -1;
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
	}

	res = SCORE_KNOWN_WIN + 40000;
	res -= 10 * distance(enemySquare,kingSquare);// devo tenere il re vicino
	res += 20 * centerDistance(enemySquare);// devo portare il re avversario vicino al bordo

	res *= sign;
	return true;

}

bool Position::_evalKRvsK(Score& res) const
{
	Color color = getBitmap(whiteRooks) ? white : black;
	tSquare kingSquare;
	tSquare enemySquare;

	if( getNumberOfLegalMoves() == 0 )
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
	res -= 10 * distance(enemySquare,kingSquare);// devo tenere il re vicino
	res += 20 * centerDistance(enemySquare);// devo portare il re avversario vicino al bordo

	res *= mul;
	return true;

}

bool Position::_kingsDirectOpposition() const
{
	if(
			(getSquareOfThePiece(whiteKing) + 16 == getSquareOfThePiece(blackKing) )
			//||
			//(getSquareOfThePiece(whiteKing) == getSquareOfThePiece(blackKing) +16 )
	)
	{
		return true;
	}
	return false;

}

bool Position::_evalKPvsK(Score& res) const
{
	Color pColor = getBitmap(whitePawns) ? white : black;
	tSquare pawnSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	tRank promotionRank;
	eNextMove turn;
	int mul;
	if(pColor == white)
	{
		pawnSquare = getSquareOfThePiece(whitePawns);
		kingSquare = getSquareOfThePiece(whiteKing);
		enemySquare = getSquareOfThePiece(blackKing);
		promotionRank = RANK8;
		turn = whiteTurn;
		mul = 1;
	}
	else
	{
		pawnSquare = getSquareOfThePiece(blackPawns);
		kingSquare = getSquareOfThePiece(blackKing);
		enemySquare = getSquareOfThePiece(whiteKing);
		promotionRank = RANK1;
		turn = blackTurn;
		mul = -1;
	}
	
	const tSquare promotionSquare = getPromotionSquareOf( pawnSquare, pColor );
	const int relativeRank = getRelativeRankOf(pawnSquare, pColor);
	// Rule of the square
	if ( std::min( 5, 7 - relativeRank) <  std::max((int)distance(enemySquare,promotionSquare) - ( isTurn(turn) ? 0 : 1) , 0) )
	{
		res = mul * (SCORE_KNOWN_WIN + relativeRank);
		return true;
	}
	if( !isLateralFile( getFileOf(pawnSquare) ) )
	{

		if(distance(enemySquare,pawnSquare) >= 2 || isTurn(turn) )
		{
			//winning king on a key square
			if(relativeRank < RANK5)
			{
				if(kingSquare >= pawnSquare + 2 * pawnPush(pColor) - 1  && kingSquare <= pawnSquare + 2 * pawnPush(pColor) + 1)
				{
					res = mul * (SCORE_KNOWN_WIN + relativeRank);
					return true;
				}
			}
			else if(relativeRank < RANK7)
			{
				if((kingSquare >= pawnSquare + 2 * pawnPush(pColor) -1 && kingSquare <= pawnSquare + 2 * pawnPush(pColor) +1) || (kingSquare >= pawnSquare + pawnPush(pColor) -1 && kingSquare <= pawnSquare + pawnPush(pColor) + 1 ))
				{
					res = mul * (SCORE_KNOWN_WIN + relativeRank);
					return true;
				}
			}
			else{

				if((kingSquare >= pawnSquare - 1 && kingSquare <= pawnSquare + 1 ) || (kingSquare >= pawnSquare + pawnPush(pColor) -1 && kingSquare <=  pawnPush(pColor) + 1 ))
				{
					res = mul * (SCORE_KNOWN_WIN + relativeRank);
					return true;
				}
			}

			// 3 rules for winning, if  conditions are met -> it's won
			unsigned int count = 0;
			if(kingSquare == pawnSquare + pawnPush(pColor) ) count++;
			if(!isTurn(turn) && _kingsDirectOpposition()) count++;
			if(getRelativeRankOf(kingSquare, pColor) == RANK6) count++;

			if(count > 1)
			{
				res = mul * (SCORE_KNOWN_WIN + relativeRank);
				return true;
			}

		}
		//draw rule
		if((enemySquare==pawnSquare+pawnPush(pColor)) || (enemySquare==pawnSquare+2*pawnPush(pColor) && getRankOf(enemySquare)!=promotionRank))
		{
			res = 0;
			return true;
		}
	}
	else
	{
		//ROOKS PAWN
		if(abs(getFileOf(enemySquare) - getFileOf(pawnSquare)) <= 1  && getRelativeRankOf(enemySquare, pColor) > RANK6 )
		{
			res = 0;
			return true;
		}
	}
	return false;
}

bool Position::_evalKPsvsK(Score& res) const
{
	Color color = getBitmap(whitePawns) ? white : black;
	
	tSquare kingSquare;
	bitMap pawns;
	
	bitMap HFile = fileMask(H1);
	bitMap AFile = fileMask(A1);
	
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
			(std::abs(getFileOf( firstOne( pawns ) ) - getFileOf( kingSquare ))<=1 )
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


bool Position::_evalOppositeBishopEndgame(Score& res) const
{
	if(isOppositeBishops())
	{
		unsigned int passedPawnCount = 0;

		bitMap pawns= getBitmap(whitePawns);
		while(pawns)
		{
			tSquare pawn = iterateBit(pawns);
			if( !(PASSED_PAWN[0][pawn] & getBitmap(blackPawns)))
			{
				++passedPawnCount;
				// todo getBishopPseudoAttack can be moved outside loop
				if(!(Movegen::getBishopPseudoAttack(getSquareOfThePiece(blackBishops)) & SQUARES_IN_FRONT_OF[0][pawn]))
				{
					res = 256;
					return true;
				}
			}
		}

		pawns= getBitmap(blackPawns);
		while(pawns)
		{
			tSquare pawn = iterateBit(pawns);
			if(!( PASSED_PAWN[1][pawn] & getBitmap(whitePawns)))
			{
				++passedPawnCount;
				// todo getBishopPseudoAttack can be moved outside loop
				if(!(Movegen::getBishopPseudoAttack(getSquareOfThePiece(whiteBishops)) & SQUARES_IN_FRONT_OF[1][pawn]))
				{
					res = 256;
					return true;
				}
			}
		}
	
		res = std::min(32 + passedPawnCount * 25, (unsigned int)256);
		return true;
	}
	return false;

}

bool Position::_evalKRvsKm(Score& res) const
{
	res = 64;
	return true;
}

bool Position::_evalKNNvsK(Score& res) const
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


	Position p(Position::nnueConfig::off, Position::pawnHash::off);
	static const struct{
		std::string fen;
		materialStruct::tType type;
		bool (Position::*pointer)(Score &) const;
		Score val;
	} Endgames[] = {
			// DRAWN
			{"k7/8/8/8/8/8/8/7K w - -",materialStruct::type::exact, nullptr, 0 },		//k vs k

			{"kb6/8/8/8/8/8/8/7K w - -",materialStruct::type::exact, nullptr, 0 },	//kb vs k
			{"k7/8/8/8/8/8/8/6BK w - -",materialStruct::type::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/7K w - -",materialStruct::type::exact, nullptr, 0 },	//kn vs k
			{"k7/8/8/8/8/8/8/6NK w - -",materialStruct::type::exact, nullptr, 0 },

			{"kn6/8/8/8/8/8/8/6NK w - -",materialStruct::type::exact, nullptr, 0 },	//kn vs kn
			{"kn6/8/8/8/8/8/8/6BK w - -",materialStruct::type::exact, nullptr, 0 },	//kn vs kb
			{"kb6/8/8/8/8/8/8/6NK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/6BK w - -",materialStruct::type::exact, nullptr, 0 },	//kb vs kb

			{"knn5/8/8/8/8/8/8/7K w - -",materialStruct::type::exact, nullptr, 0 },	//knn vs k
			{"k7/8/8/8/8/8/8/5NNK w - -",materialStruct::type::exact, nullptr, 0 },

			{"knn5/8/8/8/8/8/8/6NK w - -",materialStruct::type::exact, nullptr, 0 },
			{"knn5/8/8/8/8/8/8/6BK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kbn5/8/8/8/8/8/8/6NK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kbn5/8/8/8/8/8/8/6BK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kbb5/8/8/8/8/8/8/6NK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kbb5/8/8/8/8/8/8/6BK w - -",materialStruct::type::exact, nullptr, 0 },

			{"knp5/8/8/8/8/8/8/7K w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKNPvsK, 0 },
			{"k7/8/8/8/8/8/8/5KNP w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKNPvsK, 0 },

			{"kn6/8/8/8/8/8/8/5NNK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5NNK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/5BNK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5BNK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kn6/8/8/8/8/8/8/5BBK w - -",materialStruct::type::exact, nullptr, 0 },
			{"kb6/8/8/8/8/8/8/5BBK w - -",materialStruct::type::exact, nullptr, 0 },

			{"kb6/8/8/8/8/8/7P/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/6PP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/5PPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/4PPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/3PPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/2PPPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/1PPPPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kb6/8/8/8/8/8/PPPPPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },

			{"k7/7p/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/6pp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/5ppp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/4pppp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/3ppppp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/2pppppp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/1ppppppp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/pppppppp/8/8/8/8/8/6BK w - -",materialStruct::type::saturationH, nullptr, 0 },

			{"kn6/8/8/8/8/8/7P/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/6PP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/5PPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/4PPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/3PPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/2PPPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/1PPPPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },
			{"kn6/8/8/8/8/8/PPPPPPPP/7K w - -",materialStruct::type::saturationL, nullptr, 0 },

			{"k7/7p/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/6pp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/5ppp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/4pppp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/3ppppp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/2pppppp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/1ppppppp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },
			{"k7/pppppppp/8/8/8/8/8/6NK w - -",materialStruct::type::saturationH, nullptr, 0 },

			{"k7/8/8/8/8/8/8/5BPK w - -",materialStruct::type::exactFunction, &Position::_evalKBPsvsK, 0 },
			{"k7/8/8/8/8/8/8/4BPPK w - -",materialStruct::type::exactFunction, &Position::_evalKBPsvsK, 0 },
			{"k7/8/8/8/8/8/8/3BPPPK w - -",materialStruct::type::exactFunction, &Position::_evalKBPsvsK, 0 },
			{"kbp5/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKBPsvsK, 0 },
			{"kbpp4/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKBPsvsK, 0 },
			{"kbppp3/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKBPsvsK, 0 },
			
			{"k7/8/8/8/8/8/8/5BNK w - -",materialStruct::type::exactFunction, &Position::_evalKBNvsK, 0 },
			{"kbn5/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKBNvsK, 0 },

			{"k7/8/8/8/8/8/8/6QK w - -",materialStruct::type::exactFunction, &Position::_evalKQvsK, 0 },
			{"kq6/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKQvsK, 0 },

			{"k7/8/8/8/8/8/8/6RK w - -",materialStruct::type::exactFunction, &Position::_evalKRvsK, 0 },
			{"kr6/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKRvsK, 0 },

			{"k7/8/8/8/8/8/8/6PK w - -",materialStruct::type::exactFunction, &Position::_evalKPvsK, 0 },
			{"kp6/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPvsK, 0 },
			
			{"k7/8/8/8/8/8/8/5PPK w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/4PPPK w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/3PPPPK w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/2PPPPPK w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/1PPPPPPK w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"k7/8/8/8/8/8/8/PPPPPPPK w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"kpp5/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"kppp4/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"kpppp3/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"kppppp2/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"kpppppp1/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			{"kpppppp1/8/8/8/8/8/8/7K w - -",materialStruct::type::exactFunction, &Position::_evalKPsvsK, 0 },
			
			
			
			

			{"kr6/8/8/8/8/8/8/6NK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },
			{"kr6/8/8/8/8/8/8/6BK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },
			{"kb6/8/8/8/8/8/8/6RK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },
			{"kn6/8/8/8/8/8/8/6RK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },

			{"knn5/8/8/8/8/8/8/7K w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKNNvsK, 0 },
			{"k7/8/8/8/8/8/8/5NNK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKNNvsK, 0 },

			{"kr6/8/8/8/8/8/8/5PRK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRPvsKr, 0 },
			{"krp5/8/8/8/8/8/8/6RK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRPvsKr, 0 },
			
			{"kr6/8/8/8/8/8/8/5NRK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },
			{"krn5/8/8/8/8/8/8/6RK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },
			
			{"krn5/8/8/8/8/8/8/4NNRK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },
			{"krnn4/8/8/8/8/8/8/5NRK w - -",materialStruct::type::multiplicativeFunction, &Position::_evalKRvsKm, 0 },

			{"kq6/8/8/8/8/8/8/6PK w - -",materialStruct::type::exactFunction, &Position::_evalKQvsKP, 0 },
			{"kp6/8/8/8/8/8/8/6QK w - -",materialStruct::type::exactFunction, &Position::_evalKQvsKP, 0 }

	};

	materialStruct t;

	for (auto& eg : Endgames)
	{
		tKey key = p.setupFromFen(eg.fen).getMaterialKey().getKey();
		t.type = eg.type;
		t.pointer = eg.pointer;
		t.val = eg.val;
		_materialKeyMap.insert({key,t});
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
				tKey key = p.setupFromFen(s).getMaterialKey().getKey();
				t.type=materialStruct::type::multiplicativeFunction;
				t.pointer=&Position::_evalOppositeBishopEndgame;
				t.val=0;
				_materialKeyMap.insert({key,t});
			}
		}
	}
}
