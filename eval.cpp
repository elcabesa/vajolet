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

#include <iomanip>

#include "eval.h"
#include "io.h"
#include "parameters.h"
#include "position.h"


bool enablePawnHash= true;

static const int KingExposed[] = {
     2,  0,  2,  5,  5,  2,  0,  2,
     2,  2,  4,  8,  8,  4,  2,  2,
     7, 10, 12, 12, 12, 12, 10,  7,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15
  };


simdScore traceRes={0,0,0,0};



//---------------------------------------------
const Position::materialStruct * Position::getMaterialData()
{
	tKey key = getMaterialKey().getKey();

	auto got= materialKeyMap.find(key);

	if( got == materialKeyMap.end())
	{
		return nullptr;
	}
	else
	{
		 return &(got->second);
	}

}




template<Color c>
simdScore Position::evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const
{
	simdScore res = {0,0,0,0};

	bool passed, isolated, doubled, opposed, chain, backward;
	const bitMap ourPawns = c ? getBitmap(blackPawns) : getBitmap(whitePawns);
	const bitMap theirPawns = c ? getBitmap(whitePawns) : getBitmap(blackPawns);
	const int relativeRank = getRelativeRankOf( sq, c );

	// Our rank plus previous one. Used for chain detection
	bitMap b = rankMask(sq) | rankMask(sq - pawnPush(c));

	// Flag the pawn as passed, isolated, doubled or member of a pawn
    // chain (but not the backward one).
    chain    = (ourPawns	& ISOLATED_PAWN[sq] & b);
	isolated = !(ourPawns	& ISOLATED_PAWN[sq]);
    doubled  = (ourPawns	& SQUARES_IN_FRONT_OF[c][sq]);
    opposed  = (theirPawns	& SQUARES_IN_FRONT_OF[c][sq]);
    passed   = !(theirPawns	& PASSED_PAWN[c][sq]);

	backward = false;
	if(
		!(passed | isolated | chain) &&
		!(ourPawns & PASSED_PAWN[ 1 - c ][ sq + pawnPush( c )] & ISOLATED_PAWN[ sq ]))// non ci sono nostri pedoni dietro a noi
	{
		b = rankMask( sq + pawnPush( c )) & ISOLATED_PAWN[ sq ];
		while ( !(b & (ourPawns | theirPawns)))
		{
			if(!c)
			{
				b <<= 8;
			}
			else
			{
				b >>= 8;
			}

		}
		backward = ((b | ( ( !c ) ? ( b << 8) : ( b >> 8 ) ) ) & theirPawns );
	}

	if (isolated)
	{
		if(opposed)
		{
			res -= isolatedPawnPenaltyOpp;
		}
		else
		{
			res -= isolatedPawnPenalty;
		}
		weakPawns |= bitSet( sq );
	}

    if( doubled )
    {
    	res -= doubledPawnPenalty;
	}

    if (backward)
    {
    	if(opposed)
    	{
			res -= backwardPawnPenalty / 2;
		}
		else
		{
			res -= backwardPawnPenalty;
		}
		weakPawns |= bitSet( sq );
	}

    if(chain)
    {
		if(opposed)
    	{
			res += chainedPawnBonusOffsetOpp + chainedPawnBonusOpp * ( relativeRank - 1 ) * ( relativeRank ) ;
		}
		else
		{
			res += chainedPawnBonusOffset + chainedPawnBonus * ( relativeRank - 1 ) * ( relativeRank ) ;
		}
	}
	else
	{
		weakPawns |= bitSet( sq );
	}


	//passed pawn
	if( passed && !doubled )
	{
		passedPawns |= bitSet( sq );
	}

	if ( !passed && !isolated && !doubled && !opposed && bitCnt( PASSED_PAWN[c][sq] & theirPawns ) < bitCnt(PASSED_PAWN[c][sq-pawnPush(c)] & ourPawns ) )
	{
		res += candidateBonus * ( relativeRank - 1 );
	}
	return res;
}

template<bitboardIndex piece>
simdScore Position::evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes,bitMap const blockedPawns, bitMap * const kingRing,unsigned int * const kingAttackersCount,unsigned int * const kingAttackersWeight,unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const
{
	simdScore res = {0,0,0,0};
	bitMap tempPieces = getBitmap(piece);
	bitMap enemyKing = isBlackPiece(piece)? getBitmap(whiteKing) : getBitmap(blackKing);
	tSquare enemyKingSquare = isBlackPiece(piece)? getSquareOfThePiece(whiteKing) : getSquareOfThePiece(blackKing);
	bitMap enemyKingRing = isBlackPiece(piece) ? kingRing[white] : kingRing[black];
	unsigned int * pKingAttackersCount = isBlackPiece(piece) ? &kingAttackersCount[black] : &kingAttackersCount[white];
	unsigned int * pkingAttackersWeight = isBlackPiece(piece) ? &kingAttackersWeight[black] : &kingAttackersWeight[white];
	unsigned int * pkingAdjacentZoneAttacksCount = isBlackPiece(piece) ? &kingAdjacentZoneAttacksCount[black] : &kingAdjacentZoneAttacksCount[white];
	const bitMap & enemyWeakSquares = isBlackPiece(piece) ? weakSquares[white] : weakSquares[black];
	const bitMap & enemyHoles = isBlackPiece(piece) ? holes[white] : holes[black];
	const bitMap & supportedSquares = isBlackPiece(piece) ? attackedSquares[blackPawns] : attackedSquares[whitePawns];
	const bitMap & threatenSquares = isBlackPiece(piece) ? attackedSquares[whitePawns] : attackedSquares[blackPawns];
	const bitMap ourPieces = isBlackPiece(piece) ? getBitmap(blackPieces) : getBitmap(whitePieces);
	const bitMap ourPawns = isBlackPiece(piece) ? getBitmap(blackPawns) : getBitmap(whitePawns);
	const bitMap theirPieces = isBlackPiece(piece) ? getBitmap(whitePieces) : getBitmap(blackPieces);
	const bitMap theirPawns = isBlackPiece(piece) ? getBitmap(whitePawns) : getBitmap(blackPawns);
	const bitboardIndex pieceType = getPieceType( piece );

	(void)theirPawns;

	while(tempPieces)
	{
		tSquare sq = iterateBit(tempPieces);
		unsigned int relativeRank = getRelativeRankOf( sq, (Color)isBlackPiece(piece) );

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti

		bitMap attack;
		switch(piece)
		{
			case whiteRooks:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(whiteRooks) ^ getBitmap(whiteQueens));
				break;
			case blackRooks:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(blackRooks) ^ getBitmap(blackQueens));
				break;
			case whiteBishops:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(whiteQueens));
				break;
			case blackBishops:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap() ^ getBitmap(blackQueens));
				break;
			case whiteQueens:
			case blackQueens:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap());
				break;
			case whiteKnights:
			case blackKnights:
				attack = Movegen::attackFrom<piece>(sq, getOccupationBitmap());
				break;
			default:
				break;
		}

		if(attack & enemyKingRing)
		{
			(*pKingAttackersCount)++;
			(*pkingAttackersWeight) += KingAttackWeights[ pieceType -2 ];
			bitMap adjacent = attack & Movegen::attackFrom<whiteKing>(enemyKingSquare);
			if(adjacent)
			{
				( *pkingAdjacentZoneAttacksCount ) += bitCnt(adjacent);
			}
		}
		attackedSquares[piece] |= attack;


		bitMap defendedPieces = attack & ourPieces & ~ourPawns;
		// piece coordination
		res += (int)bitCnt( defendedPieces ) * pieceCoordination[ pieceType ];


		//unsigned int mobility = (bitCnt(attack&~(threatenSquares|ourPieces))+ bitCnt(attack&~(ourPieces)))/2;
		unsigned int mobility = bitCnt( attack & ~(threatenSquares | ourPieces));

		res += mobilityBonus[ pieceType ][ mobility ];
		if(piece != whiteKnights && piece != blackKnights)
		{
			if( !(attack & ~(threatenSquares | ourPieces) ) && isSquareSet( threatenSquares, sq ) ) // zero mobility && attacked by pawn
			{
				res -= ( pieceValue[piece] / 4 );
			}
		}
		/////////////////////////////////////////
		// center control
		/////////////////////////////////////////
		if(attack & centerBitmap)
		{
			res += (int)bitCnt(attack & centerBitmap) * piecesCenterControl[ pieceType ];
		}
		if(attack & bigCenterBitmap)
		{
			res += (int)bitCnt(attack & bigCenterBitmap) * piecesBigCenterControl[ pieceType ];
		}

		switch(piece)
		{
		case whiteQueens:
		case blackQueens:
		{
			bitMap enemyBackRank = isBlackPiece(piece) ? rankMask(A1) : rankMask(A8);
			bitMap enemyPawns = isBlackPiece(piece) ? getBitmap(whitePawns) : getBitmap(blackPawns);
			//--------------------------------
			// donna in 7a con re in 8a
			//--------------------------------
			if(relativeRank == RANK7 && (enemyKing & enemyBackRank) )
			{
				res += queenOn7Bonus;
			}
			//--------------------------------
			// donna su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank > RANK5 && (rankMask(sq) & enemyPawns))
			{
				res += queenOnPawns;
			}
			break;
		}
		case whiteRooks:
		case blackRooks:
		{
			bitMap enemyBackRank = isBlackPiece(piece) ? rankMask(A1) : rankMask(A8);
			bitMap enemyPawns = isBlackPiece(piece) ?  getBitmap(whitePawns) : getBitmap(blackPawns);
			bitMap ourPawns = isBlackPiece(piece) ? getBitmap(blackPawns) : getBitmap(whitePawns);
			//--------------------------------
			// torre in 7a con re in 8a
			//--------------------------------
			if(relativeRank == RANK7 && (enemyKing & enemyBackRank) )
			{
				res += rookOn7Bonus;
			}
			//--------------------------------
			// torre su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank > RANK5 && (rankMask(sq) & enemyPawns))
			{
				res += rookOnPawns;
			}
			//--------------------------------
			// torre su colonna aperta/semiaperta
			//--------------------------------
			if( !(fileMask(sq) & ourPawns) )
			{
				if( !(fileMask(sq) & enemyPawns) )
				{
					res += rookOnOpen;
				}else
				{
					res += rookOnSemi;
				}
			}
			//--------------------------------
			// torre intrappolata
			//--------------------------------
			else if( mobility < 3 )
			{

				tSquare ksq = isBlackPiece(piece) ?  getSquareOfThePiece(blackKing) : getSquareOfThePiece(whiteKing);
				unsigned int relativeRankKing = getRelativeRankOf( ksq, (Color)isBlackPiece(piece) );

				if(
					((getFileOf(ksq) < FILEE) == (getFileOf(sq) < getFileOf(ksq))) &&
					(getRankOf(ksq) == getRankOf(sq) && relativeRankKing == RANK1)
				)
				{

					res -= rookTrapped*(int)(3-mobility);
					const state & st = getActualStateConst();
					if( isBlackPiece(piece) )
					{
						if( !st.hasCastleRight( bCastleOO | bCastleOOO ) )
						{
							res -= rookTrappedKingWithoutCastling * (int)( 3 - mobility );
						}

					}
					else
					{
						if( !st.hasCastleRight( wCastleOO | wCastleOOO ) )
						{
							res -= rookTrappedKingWithoutCastling * (int)( 3 - mobility ) ;
						}
					}
				}
			}
			break;
		}
		case whiteBishops:
		case blackBishops:
			if( relativeRank >= 4 && isSquareSet( enemyWeakSquares, sq ) )
			{
				res += bishopOnOutpost;
				if( isSquareSet( supportedSquares, sq ) )
				{
					res += bishopOnOutpostSupported;
				}
				if( isSquareSet( enemyHoles, sq ) )
				{
					res += bishopOnHole;
				}

			}
			// alfiere cattivo
			{
				Color color = getSquareColor(sq);
				bitMap blockingPawns = ourPieces & blockedPawns & getColorBitmap(color);
				if( moreThanOneBit(blockingPawns) )
				{
					res -= (int)bitCnt(blockingPawns) * badBishop;
				}
			}

			break;
		case whiteKnights:
		case blackKnights:
			if( isSquareSet( enemyWeakSquares, sq ) )
			{
				res += knightOnOutpost * ( 5 - std::abs( (int)relativeRank - 5 ));
				if( isSquareSet( supportedSquares, sq ) )
				{
					res += knightOnOutpostSupported;
				}
				if( isSquareSet( enemyHoles, sq ) )
				{
					res += knightOnHole;
				}

			}

			{
				bitMap wpa = attack & (weakPawns) & theirPieces;
				if(wpa)
				{
					res += (int)bitCnt(wpa) * KnightAttackingWeakPawn;
				}
			}
			break;
		default:
			break;
		}
	}
	return res;
}

template<Color kingColor>
Score Position::evalShieldStorm(tSquare ksq) const
{
	if( getFileOf(ksq) == FILEA )
	{
		ksq ++;
	}
	if( getFileOf(ksq) == FILEH )
	{
		ksq --;
	}
	Score ks = 0;
	const bitMap ourPawns = kingColor ? getBitmap(blackPawns) : getBitmap(whitePawns);
	const bitMap theirPawns = kingColor ? getBitmap(whitePawns) : getBitmap(blackPawns);
	const unsigned int disableRank= kingColor ? 0: 7;
	bitMap localKingRing = Movegen::attackFrom<whiteKing>(ksq);
	bitMap localKingShield = localKingRing;

	if(getRankOf(ksq) != disableRank)
	{
		localKingRing |= Movegen::attackFrom<whiteKing>(ksq + pawnPush(kingColor));
	}
	bitMap localKingFarShield = localKingRing & ~(localKingShield);

	bitMap pawnShield = localKingShield & ourPawns;
	bitMap pawnFarShield = localKingFarShield & ourPawns;
	bitMap pawnStorm = PASSED_PAWN[kingColor][ksq] & theirPawns;
	if(pawnShield)
	{
		ks = bitCnt(pawnShield) * kingShieldBonus[0];
	}
	if(pawnFarShield)
	{
		ks += bitCnt(pawnFarShield) * kingFarShieldBonus[0];
	}
	while(pawnStorm)
	{
		tSquare p = iterateBit(pawnStorm);
		ks -= ( 7 - distance(p,ksq) ) * kingStormBonus[0];
	}
	return ks;
}

template<Color c>
simdScore Position::evalPassedPawn(bitMap pp, bitMap* attackedSquares) const
{
	tSquare kingSquare = c ? getSquareOfThePiece( blackKing ) : getSquareOfThePiece( whiteKing );
	tSquare enemyKingSquare = c ?getSquareOfThePiece( whiteKing ) : getSquareOfThePiece( blackKing );
	bitboardIndex ourPieces = c ? blackPieces : whitePieces;
	bitboardIndex enemyPieces = c ? whitePieces : blackPieces;
	bitboardIndex ourRooks = c ? blackRooks : whiteRooks;
	bitboardIndex enemyRooks = c ? whiteRooks : blackRooks;
	bitboardIndex ourPawns = c ? blackPawns : whitePawns;
	const state st = getActualStateConst();

	simdScore score = {0,0,0,0};
	while(pp)
	{
		
		simdScore passedPawnsBonus;
		tSquare ppSq = iterateBit(pp);
		unsigned int relativeRank = getRelativeRankOf( ppSq, c );
		
		int r = relativeRank - 1;
		int rr =  r * ( r - 1 );
		int rrr =  r * r * r;
		

		passedPawnsBonus = simdScore{ passedPawnBonus[0] * rrr, passedPawnBonus[1] * ( rrr ), 0, 0};
		
		bitMap forwardSquares = c ? SQUARES_IN_FRONT_OF[black][ppSq] : SQUARES_IN_FRONT_OF[white][ppSq];
		bitMap unsafeSquares = forwardSquares & (attackedSquares[enemyPieces] | getBitmap(enemyPieces) );
		passedPawnsBonus -= passedPawnUnsafeSquares * (int)bitCnt(unsafeSquares);
		
		//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
		if(rr)
		{
			tSquare blockingSquare = ppSq + pawnPush(c);

			// bonus for king proximity to blocking square
			passedPawnsBonus += enemyKingNearPassedPawn * int( distance( blockingSquare, enemyKingSquare ) * rr );
			passedPawnsBonus -= ownKingNearPassedPawn * int( distance( blockingSquare, kingSquare ) * rr );
			//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
			if( getPieceAt(blockingSquare) == empty )
			{
				
				bitMap backWardSquares = c ? SQUARES_IN_FRONT_OF[white][ppSq] : SQUARES_IN_FRONT_OF[black][ppSq];

				bitMap defendedSquares = forwardSquares & attackedSquares[ ourPieces ];

			
				if ( isSquareSet( unsafeSquares, blockingSquare ) )
				{
					passedPawnsBonus -= passedPawnBlockedSquares * rr;
				}

				//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
				if(defendedSquares)
				{
					passedPawnsBonus += passedPawnDefendedSquares * rr * (int)bitCnt( defendedSquares );
					if( isSquareSet( defendedSquares, blockingSquare ) )
					{
						passedPawnsBonus += passedPawnDefendedBlockingSquare * rr;
					}
				}
				//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
				if(backWardSquares & getBitmap( ourRooks ))
				{
					passedPawnsBonus += rookBehindPassedPawn * rr;
				}
				if(backWardSquares & getBitmap( enemyRooks ))
				{
					passedPawnsBonus -= EnemyRookBehindPassedPawn * rr;
				}
				//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
			}
		}

		if( isLateralFile( getFileOf( ppSq ) ) )
		{
			passedPawnsBonus -= passedPawnFileAHPenalty;
		}

		bitMap supportingPawns = getBitmap( ourPawns ) & ISOLATED_PAWN[ ppSq ];
		if( supportingPawns & rankMask(ppSq) )
		{
			passedPawnsBonus+=passedPawnSupportedBonus*rr;
		}
		if( supportingPawns & rankMask( ppSq - pawnPush(c) ) )
		{
			passedPawnsBonus += passedPawnSupportedBonus * ( rr / 2 );
		}
		//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;

		if( st.getNonPawnValue()[ c ? 0 : 2 ] == 0 )
		{
			tSquare promotionSquare = getPromotionSquareOf( ppSq, c );
			if( std::min( 5, (int)(7- relativeRank)) <  std::max((int)distance( enemyKingSquare, promotionSquare ) - ( (c ? isBlackTurn() : isWhiteTurn() ) ? 0 : 1 ), 0) )
			{
				passedPawnsBonus += unstoppablePassed * rr;
			}
		}
		//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;


		score += passedPawnsBonus;

	}
	return score;
}


template<Color c> simdScore Position::evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const
{
	//sync_cout<<"kingSafety color "<<c<<sync_endl;
	simdScore res = {0,0,0,0};
	bitMap AttackingPieces = c ? getBitmap(whitePieces) : getBitmap(blackPieces);
	bitMap * DefendedSquaresBy = c ? &attackedSquares[blacks] : &attackedSquares[whites];
	bitMap * AttackedSquaresBy = c ? &attackedSquares[whites] : &attackedSquares[blacks];
	tSquare kingSquare = c ? getSquareOfThePiece(blackKing) : getSquareOfThePiece(whiteKing);
	//sync_cout<<" kingAttackersCount "<<kingAttackersCount<<sync_endl;
	if( kingAttackersCount )// se il re e' attaccato
	{

		bitMap undefendedSquares = AttackedSquaresBy[Pieces] & DefendedSquaresBy[King];
		//displayBitmap(undefendedSquares);
		undefendedSquares &=
			~( DefendedSquaresBy[Pawns]
			| DefendedSquaresBy[Knights]
			| DefendedSquaresBy[Bishops]
			| DefendedSquaresBy[Rooks]
			/*| DefendedSquaresBy[Queens]*/);
		//displayBitmap(undefendedSquares);
		bitMap undefendedSquares2 = ~ DefendedSquaresBy[Pieces];
		
		signed int attackUnits = kingAttackersCount * kingAttackersWeight;
		//sync_cout<<" attackUnits1 "<<attackUnits<<sync_endl;
		attackUnits += kingAdjacentZoneAttacksCount * kingSafetyPars1[0];
		//sync_cout<<" attackUnits2 "<<attackUnits<<sync_endl;
		attackUnits += bitCnt( undefendedSquares ) * kingSafetyPars1[1];
		//sync_cout<<" attackUnits3 "<<attackUnits<<sync_endl;
		attackUnits += KingExposed[ c? 63 - kingSquare : kingSquare ] * kingSafetyPars1[2];
		//sync_cout<<" attackUnits4 "<<attackUnits<<sync_endl;
		attackUnits -= (getPieceCount(c? whiteQueens: blackQueens)==0) * kingSafetyPars1[3];
		//sync_cout<<" attackUnits5 "<<attackUnits<<sync_endl;

		attackUnits -= 10 * kingSafety / kingStormBonus[1] ;
		//sync_cout<<" attackUnits6 "<<attackUnits<<sync_endl;
		attackUnits += kingStormBonus[2];
		//sync_cout<<" attackUnits7 "<<attackUnits<<sync_endl;
	



		// long distance check
		bitMap rMap = Movegen::attackFrom<whiteRooks>( kingSquare, getOccupationBitmap() );
		bitMap bMap = Movegen::attackFrom<whiteBishops>( kingSquare, getOccupationBitmap() );

		if(  (rMap | bMap) & AttackedSquaresBy[Queens] &  ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += kingSafetyPars2[0];
		}
		if(  rMap & AttackedSquaresBy[Rooks] &  ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += kingSafetyPars2[1];
		}
		if(  bMap & AttackedSquaresBy[Bishops] &  ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += kingSafetyPars2[2];
		}
		if( Movegen::attackFrom<whiteKnights>( kingSquare ) & ( AttackedSquaresBy[Knights] ) & ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += kingSafetyPars2[3];
		}

		attackUnits = std::max( 0, attackUnits );
		//sync_cout<<" attackUnits8 "<<attackUnits<<sync_endl;
		simdScore ks = {attackUnits * attackUnits / kingSafetyBonus[0], 10 * attackUnits / kingSafetyBonus[1], 0, 0 };
		//sync_cout<<" MG "<<attackUnits * attackUnits / kingSafetyBonus[0]<<sync_endl;
		//sync_cout<<" EG "<< 10 * attackUnits / kingSafetyBonus[1]<<sync_endl;
		
		res = -ks;
	}
	return res;
}

/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
template<bool trace>
Score Position::eval(void)
{

	const state &st = getActualState();

	if(trace)
	{

		sync_cout << std::setprecision(3) << std::setw(21) << "Eval term " << "|     White    |     Black    |      Total     \n"
			      <<             "                     |    MG    EG  |   MG     EG  |   MG      EG   \n"
			      <<             "---------------------+--------------+--------------+-----------------" << sync_endl;
	}

	Score lowSat = -SCORE_INFINITE;
	Score highSat = SCORE_INFINITE;
	Score mulCoeff = 256;

	bitMap attackedSquares[lastBitboard] = { 0 };
	bitMap weakSquares[2];
	bitMap holes[2];
	bitMap kingRing[2];
	bitMap kingShield[2];
	bitMap kingFarShield[2];
	unsigned int kingAttackersCount[2] = {0};
	unsigned int kingAttackersWeight[2] = {0};
	unsigned int kingAdjacentZoneAttacksCount[2] = {0};

	tSquare k = getSquareOfThePiece(whiteKing);
	if( getFileOf(k) == FILEA )
	{
		k ++;
	}
	if( getFileOf(k) == FILEH )
	{
		k --;
	}
	kingRing[white] = Movegen::attackFrom<whiteKing>(k);
	kingShield[white] = kingRing[white];
	if( getRankOf(k) < RANK8 )
	{
		kingRing[white] |= Movegen::attackFrom<whiteKing>( tSquare( k + 8) );
	}
	kingFarShield[white] = kingRing[white] & ~( kingShield[white] | bitSet( k ) );


	k = getSquareOfThePiece(blackKing);
	if( getFileOf(k) == FILEA )
	{
		k ++;
	}
	if( getFileOf(k) == FILEH )
	{
		k --;
	}
	kingRing[black] = Movegen::attackFrom<whiteKing>(k);
	kingShield[black] = kingRing[black];
	if( getRankOf(k) > RANK1 )
	{
		kingRing[black] |= Movegen::attackFrom<whiteKing>( tSquare( k - 8 ) );
	}
	kingFarShield[black] = kingRing[black] & ~( kingShield[black] | bitSet( k ) );

	// todo modificare valori material value & pst
	// material + pst

	simdScore res = st.getMaterialValue();


	//-----------------------------------------------------
	//	material evalutation
	//-----------------------------------------------------


	const materialStruct* materialData = getMaterialData();
	if( materialData )
	{
		bool (Position::*pointer)(Score &) = materialData->pointer;
		switch(materialData->type)
		{
			case materialStruct::exact:
				return isBlackTurn() ? -materialData->val : materialData->val;
				break;
			case materialStruct::multiplicativeFunction:
			{
				Score r;
				if( (this->*pointer)(r))
				{
					mulCoeff = r;
				}
				break;
			}
			case materialStruct::exactFunction:
			{
				Score r = 0;
				if( (this->*pointer)(r))
				{
					return isBlackTurn() ? -r : r;
				}
				break;
			}
			case materialStruct::saturationH:
				highSat = materialData->val;
				break;
			case materialStruct::saturationL:
				lowSat = materialData->val;
				break;
		}
	}
	else
	{
		// analize k and pieces vs king
		if( (bitCnt(getBitmap(whitePieces) )== 1 && bitCnt(getBitmap(blackPieces) )> 1) || (bitCnt(getBitmap(whitePieces) )> 1 && bitCnt(getBitmap(blackPieces) )== 1) )
		{
			Score r;
			evalKxvsK(r);
			return isBlackTurn() ? -r : r;
		}
	}


	//---------------------------------------------
	//	tempo
	//---------------------------------------------
	res += isBlackTurn() ? -tempo : tempo;

	if(trace)
	{
		sync_cout << std::setw(20) << "Material, PST, Tempo" << " |   ---    --- |   ---    --- | "
		          << std::setw(6)  << res[0]/10000.0 << " "
		          << std::setw(6)  << res[1]/10000.0 << " "<< sync_endl;
		traceRes = res;
	}


	//---------------------------------------------
	//	imbalancies
	//---------------------------------------------
	//	bishop pair

	if( getPieceCount(whiteBishops) >= 2 )
	{
		if( (getBitmap(whiteBishops) & getColorBitmap(white)) && (getBitmap(whiteBishops) & getColorBitmap(black) ) )
		{
			res += bishopPair;
		}
	}

	if( getPieceCount(blackBishops) >= 2 )
	{
		if( (getBitmap(blackBishops) & getColorBitmap(white)) && (getBitmap(blackBishops) & getColorBitmap(black) ) )
		{
			res -= bishopPair;
		}
	}
	if( getPieceCount(blackPawns) + getPieceCount(whitePawns) == 0 )
	{
		if((int)getPieceCount(whiteQueens) - (int)getPieceCount(blackQueens) == 1
				&& (int)getPieceCount(blackRooks) - (int)getPieceCount(whiteRooks) == 1
				&& (int)getPieceCount(blackBishops) + (int)getPieceCount(blackKnights) - (int)getPieceCount(whiteBishops) - (int)getPieceCount(whiteKnights) == 2)
		{
			res += queenVsRook2MinorsImbalance;

		}
		else if((int)getPieceCount(whiteQueens) - (int)getPieceCount(blackQueens) == -1
				&& (int)getPieceCount(blackRooks) - (int)getPieceCount(whiteRooks) == -1
				&& (int)getPieceCount(blackBishops) + (int)getPieceCount(blackKnights) - (int)getPieceCount(whiteBishops) -(int)getPieceCount(whiteKnights) == -2)
		{
			res -= queenVsRook2MinorsImbalance;

		}
	}

	if(trace)
	{
		sync_cout << std::setw(20) << "imbalancies" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}


	//todo specialized endgame & scaling function
	//todo material imbalance
	bitMap weakPawns = 0;
	bitMap passedPawns = 0;




	//----------------------------------------------
	//	PAWNS EVALUTATION
	//----------------------------------------------
	simdScore pawnResult;
	const HashKey& pawnKey = getPawnKey();
	pawnEntry& probePawn = pawnHashTable.probe( pawnKey );
	if( enablePawnHash && ( probePawn.key == pawnKey ) )
	{
		pawnResult = simdScore{probePawn.res[0], probePawn.res[1], 0, 0};
		weakPawns = probePawn.weakPawns;
		passedPawns = probePawn.passedPawns;
		attackedSquares[whitePawns] = probePawn.pawnAttacks[0];
		attackedSquares[blackPawns] = probePawn.pawnAttacks[1];
		weakSquares[white] = probePawn.weakSquares[0];
		weakSquares[black] = probePawn.weakSquares[1];
		holes[white] = probePawn.holes[0];
		holes[black] = probePawn.holes[1];
	}
	else
	{


		pawnResult = simdScore{0,0,0,0};
		bitMap pawns = getBitmap(whitePawns);

		while(pawns)
		{
			tSquare sq = iterateBit(pawns);
			pawnResult += evalPawn<white>(sq, weakPawns, passedPawns);
		}

		pawns = getBitmap(blackPawns);

		while(pawns)
		{
			tSquare sq = iterateBit(pawns);
			pawnResult -= evalPawn<black>(sq, weakPawns, passedPawns);
		}



		bitMap temp = getBitmap(whitePawns);
		bitMap pawnAttack = (temp & ~fileMask(H1) ) << 9;
		pawnAttack |= (temp & ~fileMask(A1) ) << 7;

		attackedSquares[whitePawns] = pawnAttack;
		pawnAttack |= pawnAttack << 8;
		pawnAttack |= pawnAttack << 16;
		pawnAttack |= pawnAttack << 32;

		weakSquares[white] = ~pawnAttack;


		temp = getBitmap(blackPawns);
		pawnAttack = ( temp & ~fileMask(H1) ) >> 7;
		pawnAttack |= ( temp & ~fileMask(A1) ) >> 9;

		attackedSquares[blackPawns] = pawnAttack;

		pawnAttack |= pawnAttack >> 8;
		pawnAttack |= pawnAttack >> 16;
		pawnAttack |= pawnAttack >> 32;

		weakSquares[black] = ~pawnAttack;

		temp = getBitmap(whitePawns) << 8;
		temp |= temp << 8;
		temp |= temp << 16;
		temp |= temp << 32;

		holes[white] = weakSquares[white] & temp;



		temp = getBitmap(blackPawns) >> 8;
		temp |= temp >> 8;
		temp |= temp >> 16;
		temp |= temp >> 32;

		holes[black] = weakSquares[black] & temp;
		pawnResult -= ( (int)bitCnt( holes[white] ) - (int)bitCnt( holes[black] ) ) * holesPenalty;

		if(enablePawnHash)
		{
			pawnHashTable.insert(pawnKey, pawnResult, weakPawns, passedPawns, attackedSquares[whitePawns], attackedSquares[blackPawns], weakSquares[white], weakSquares[black], holes[white], holes[black] );
		}

	}

	res += pawnResult;
	//---------------------------------------------
	// center control
	//---------------------------------------------

	if( attackedSquares[whitePawns] & centerBitmap )
	{
		res += (int)bitCnt( attackedSquares[whitePawns] & centerBitmap ) * pawnCenterControl;
	}
	if( attackedSquares[whitePawns] & bigCenterBitmap )
	{
		res += (int)bitCnt( attackedSquares[whitePawns] & bigCenterBitmap )* pawnBigCenterControl;
	}

	if( attackedSquares[blackPawns] & centerBitmap )
	{
		res -= (int)bitCnt( attackedSquares[blackPawns] & centerBitmap ) * pawnCenterControl;
	}
	if( attackedSquares[blackPawns] & bigCenterBitmap )
	{
		res -= (int)bitCnt( attackedSquares[blackPawns] & bigCenterBitmap ) * pawnBigCenterControl;
	}

	if(trace)
	{
		sync_cout << std::setw(20) << "pawns" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}


	//-----------------------------------------
	//	blocked pawns
	//-----------------------------------------
	bitMap blockedPawns = ( getBitmap(whitePawns)<<8 ) & getBitmap( blackPawns );
	blockedPawns |= blockedPawns >> 8;
	
	
	//-----------------------------------------
	//	add pawn to king attackers
	//-----------------------------------------
	kingAttackersCount[white] = bitCnt(kingRing[black] & attackedSquares[whitePawns]);
	kingAttackersCount[black] = bitCnt(kingRing[white] & attackedSquares[blackPawns]);

	//-----------------------------------------
	//	pieces
	//-----------------------------------------

	simdScore wScore;
	simdScore bScore;
	wScore = evalPieces<whiteKnights>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<blackKnights>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	res += wScore - bScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "knights" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}

	wScore = evalPieces<whiteBishops>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<blackBishops>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	res += wScore - bScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "bishops" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}

	wScore = evalPieces<whiteRooks>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<blackRooks>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	res += wScore - bScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "rooks" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes=res;
	}

	wScore = evalPieces<whiteQueens>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = evalPieces<blackQueens>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	res += wScore - bScore;

	if(trace)
	{
		sync_cout << std::setw(20) << "queens" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}


	attackedSquares[whiteKing] = Movegen::attackFrom<whiteKing>(getSquareOfThePiece(whiteKing));
	attackedSquares[blackKing] = Movegen::attackFrom<blackKing>(getSquareOfThePiece(blackKing));

	attackedSquares[whitePieces] = attackedSquares[whiteKing]
								| attackedSquares[whiteKnights]
								| attackedSquares[whiteBishops]
								| attackedSquares[whiteRooks]
								| attackedSquares[whiteQueens]
								| attackedSquares[whitePawns];

	attackedSquares[blackPieces] = attackedSquares[blackKing]
								| attackedSquares[blackKnights]
								| attackedSquares[blackBishops]
								| attackedSquares[blackRooks]
								| attackedSquares[blackQueens]
								| attackedSquares[blackPawns];


	//-----------------------------------------
	//	passed pawn evalutation
	//-----------------------------------------


	wScore = evalPassedPawn<white>(passedPawns & getBitmap( whitePawns ), attackedSquares);
	bScore = evalPassedPawn<black>(passedPawns & getBitmap( blackPawns ), attackedSquares);
	res += wScore - bScore;

	if(trace)
	{
		sync_cout << std::setw(20) << "passed pawns" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0]) / 10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1]) / 10000.0 << " " << sync_endl;
		traceRes=res;
	}
	//todo attacked squares

	//---------------------------------------
	//	space
	//---------------------------------------
	// white pawns
	bitMap spacew = getBitmap(whitePawns) & spaceMask;
	spacew |= spacew >> 8;
	spacew |= spacew >> 16;
	spacew |= spacew >> 32;
	spacew &= ~attackedSquares[blackPieces];

	// black pawns
	bitMap spaceb = getBitmap(blackPawns) & spaceMask;
	spaceb |= spaceb << 8;
	spaceb |= spaceb << 16;
	spaceb |= spaceb << 32;
	spaceb &= ~attackedSquares[whitePieces];

	res += ( (int)bitCnt(spacew) - (int)bitCnt(spaceb) ) * spaceBonus;

	if(trace)
	{
		wScore = (int)bitCnt(spacew) * spaceBonus;
		bScore = (int)bitCnt(spaceb) * spaceBonus;
		sync_cout << std::setw(20) << "space" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}


	//todo counterattack??

	//todo weakpawn

	//--------------------------------------
	//	weak pieces
	//--------------------------------------
	wScore = simdScore{0,0,0,0};
	bScore = simdScore{0,0,0,0};

	bitMap pawnAttackedPieces = getBitmap( whitePieces ) & attackedSquares[ blackPawns ];
	while(pawnAttackedPieces)
	{
		tSquare attacked = iterateBit( pawnAttackedPieces );
		wScore -= attackedByPawnPenalty[ getPieceTypeAt(attacked) ];
	}

	// todo fare un weak piece migliore:qualsiasi pezzo attaccato riceve un malus dipendente dal suo più debole attaccante e dal suo valore.
	// volendo anche da quale pezzo è difeso
	bitMap undefendedMinors =  (getBitmap(whiteKnights) | getBitmap(whiteBishops))  & ~attackedSquares[whitePieces];
	if (undefendedMinors)
	{
		wScore -= undefendedMinorPenalty;
	}
	bitMap weakPieces = getBitmap(whitePieces) & attackedSquares[blackPieces] & ~attackedSquares[whitePawns];
	while(weakPieces)
	{
		tSquare sq = iterateBit(weakPieces);

		bitboardIndex attackedPieceType = getPieceTypeAt(sq);
		bitboardIndex attackingPiece = blackPawns;
		for(; attackingPiece >= blackKing; attackingPiece = (bitboardIndex)(attackingPiece - 1) )
		{
			if( isSquareSet( attackedSquares[ attackingPiece ], sq ) )
			{
				wScore -= weakPiecePenalty[ attackedPieceType ][ getPieceType( attackingPiece ) ];
				break;
			}
		}

	}
	weakPieces =   getBitmap(whitePawns)
		& ~attackedSquares[whitePieces]
		& attackedSquares[blackKing];

	if(weakPieces)
	{
		wScore -= weakPawnAttackedByKing;
	}

	pawnAttackedPieces = getBitmap( blackPieces ) & attackedSquares[ whitePawns ];
	while(pawnAttackedPieces)
	{
		tSquare attacked = iterateBit( pawnAttackedPieces );
		bScore -= attackedByPawnPenalty[ getPieceTypeAt(attacked) ];
	}

	undefendedMinors =  (getBitmap(blackKnights) | getBitmap(blackBishops))  & ~attackedSquares[blackPieces];
	if (undefendedMinors)
	{
		bScore -= undefendedMinorPenalty;
	}
	weakPieces = getBitmap(blackPieces) & attackedSquares[whitePieces] & ~attackedSquares[blackPawns];
	while(weakPieces)
	{
		tSquare sq = iterateBit(weakPieces);

		bitboardIndex attackedPieceType = getPieceTypeAt(sq);
		bitboardIndex attackingPiece = whitePawns;
		for(; attackingPiece >= whiteKing; attackingPiece = (bitboardIndex)(attackingPiece - 1) )
		{
			if( isSquareSet( attackedSquares[ attackingPiece ], sq ) )
			{
				bScore -= weakPiecePenalty[ attackedPieceType ][ getPieceType(attackingPiece) ];
				break;
			}
		}
	}

	weakPieces =   getBitmap(blackPawns)
		& ~attackedSquares[blackPieces]
		& attackedSquares[whiteKing];

	if(weakPieces)
	{
		bScore -= weakPawnAttackedByKing;
	}




	// trapped pieces
	// rook trapped on first rank by own king



	res += wScore - bScore;
	if(trace)
	{
		sync_cout << std::setw(20) << "threat" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}

	//--------------------------------------
	//	king safety
	//--------------------------------------
	Score kingSafety[2] = {0, 0};

	kingSafety[white] = evalShieldStorm<white>(getSquareOfThePiece(whiteKing));

	if( st.hasCastleRight( wCastleOO )
		&& !(attackedSquares[blackPieces] & (bitSet(E1) | bitSet(F1) | bitSet(G1) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(F1) | bitSet(G1))) <= 1
		)
	{
		kingSafety[white] = std::max( evalShieldStorm<white>(G1), kingSafety[white]);
	}

	if( st.hasCastleRight( wCastleOOO )
		&& !(attackedSquares[blackPieces] & (bitSet(E1) | bitSet(D1) | bitSet(C1) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(D1) | bitSet(C1) | bitSet(B1) )) <=1
		)
	{
		kingSafety[white] = std::max( evalShieldStorm<white>(C1), kingSafety[white]);
	}
	if(trace)
	{
		wScore = simdScore{ kingSafety[white], 0, 0, 0};
	}

	kingSafety[black] = evalShieldStorm<black>(getSquareOfThePiece(blackKing));

	if( st.hasCastleRight( bCastleOO )
		&& !(attackedSquares[whitePieces] & (bitSet(E8) | bitSet(F8) | bitSet(G8) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(F8) | bitSet(G8))) <=1
		)
	{
		kingSafety[black] = std::max( evalShieldStorm<black>(G8), kingSafety[black]);
	}

	if( st.hasCastleRight( bCastleOOO )
		&& !(attackedSquares[whitePieces] & (bitSet(E8) | bitSet(D8) | bitSet(C8) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(D8) | bitSet(C8) | bitSet(B8))) <=1
		)
	{
		kingSafety[black] = std::max(evalShieldStorm<black>(C8), kingSafety[black]);
	}
	if(trace)
	{
		bScore = simdScore{ kingSafety[black], 0, 0, 0};
	}

	res+=simdScore{kingSafety[white]-kingSafety[black],0,0,0};
	
	//sync_cout<<kingSafety[white]<<sync_endl;
	//sync_cout<<kingSafety[black]<<sync_endl;




	simdScore kingSaf = evalKingSafety<white>(kingSafety[white], kingAttackersCount[black], kingAdjacentZoneAttacksCount[black], kingAttackersWeight[black], attackedSquares);

	res += kingSaf;
	if(trace)
	{
		//sync_cout<<kingSaf[0]<<sync_endl;
		//sync_cout<<kingSaf[1]<<sync_endl;
		wScore += kingSaf;
	}

	kingSaf = evalKingSafety<black>(kingSafety[black], kingAttackersCount[white], kingAdjacentZoneAttacksCount[white], kingAttackersWeight[white], attackedSquares);

	res-= kingSaf;
	if(trace)
	{
		//sync_cout<<kingSaf[0]<<sync_endl;
		//sync_cout<<kingSaf[1]<<sync_endl;
		bScore += kingSaf;
	}

	if(trace)
	{
		sync_cout << std::setw(20) << "king safety" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0] - traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1] - traceRes[1])/10000.0 << " "<< sync_endl;
		sync_cout << "---------------------+--------------+--------------+-----------------" << sync_endl;
		sync_cout << std::setw(20) << "result" << " |"
				  << std::setw(6)  << " " << " "
				  << std::setw(6)  << " "<< " |"
				  << std::setw(6)  << " "<< " "
				  << std::setw(6)  << " " << " | "
				  << std::setw(6)  << (res[0])/10000.0 << " "
				  << std::setw(6)  << (res[1])/10000.0 << " " << sync_endl;
		traceRes = res;
	}

	//todo scaling
	const simdScore& npv = st.getNonPawnValue();
	if(mulCoeff == 256 && (getPieceCount(whitePawns) + getPieceCount(blackPawns) == 0 ) && (abs( st.getMaterialValue()[0] )< 40000) )
	{
		//Score sumMaterial = st.nonPawnMaterial[0] + st.nonPawnMaterial[2];
		//mulCoeff = std::max(std::min((Score) (sumMaterial* 0.0003 - 14), 256), 40);
		if( (npv[0]< 90000) && (npv[2] < 90000) )
		{
			mulCoeff = 40;
		}
		//else
		//{
		//	mulCoeff = 65;
		//}

	}

	if(mulCoeff == 256  && npv[0] + npv[2] < 40000  &&  (npv[0] + npv[2] !=0) && (getPieceCount(whitePawns) == getPieceCount(blackPawns)) && !passedPawns )
	{
		mulCoeff = std::min((unsigned int)256, getPieceCount(whitePawns) * 80);
	}







	//--------------------------------------
	//	finalizing
	//--------------------------------------
	signed int gamePhase = getGamePhase( st );
	signed long long r = ( (signed long long)res[0] ) * ( 65536 - gamePhase ) + ( (signed long long)res[1] ) * gamePhase;

	Score score = (Score)( (r) / 65536 );
	if(mulCoeff != 256)
	{
		score *= mulCoeff;
		score /= 256;
	}

	// final value saturation
	score = std::min(highSat,score);
	score = std::max(lowSat,score);

	return isBlackTurn() ? -score : score;

}

template Score Position::eval<false>(void);
template Score Position::eval<true>(void);
