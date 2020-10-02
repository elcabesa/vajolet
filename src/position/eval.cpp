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

#include "nnue.h"
#include "parameters.h"
#include "pawnTable.h"
#include "position.h"
#include "vajo_io.h"


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
const Position::materialStruct * Position::_getMaterialData() const
{
	tKey key = getMaterialKey().getKey();

	auto got= _materialKeyMap.find(key);

	if( got == _materialKeyMap.end())
	{
		return nullptr;
	}
	else
	{
		 return &(got->second);
	}

}




template<Color c>
simdScore Position::_evalPawn(tSquare sq, bitMap& weakPawns, bitMap& passedPawns) const
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
			res -= _eParm.isolatedPawnPenaltyOpp;
		}
		else
		{
			res -= _eParm.isolatedPawnPenalty;
		}
		weakPawns |= bitSet( sq );
	}

	if( doubled )
	{
		res -= _eParm.doubledPawnPenalty;
	}

	if (backward)
	{
		if(opposed)
		{
			res -= _eParm.backwardPawnPenalty / 2;
		}
		else
		{
			res -= _eParm.backwardPawnPenalty;
		}
		weakPawns |= bitSet( sq );
	}

	if(chain)
	{
		if(opposed)
		{
			res += _eParm.chainedPawnBonusOffsetOpp + _eParm.chainedPawnBonusOpp * ( relativeRank - 1 ) * ( relativeRank ) ;
		}
		else
		{
			res += _eParm.chainedPawnBonusOffset + _eParm.chainedPawnBonus * ( relativeRank - 1 ) * ( relativeRank ) ;
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
		res += _eParm.candidateBonus * ( relativeRank - 1 );
	}
	return res;
}

template<bitboardIndex piece>
simdScore Position::_evalPieces(const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes,bitMap const blockedPawns, bitMap * const kingRing,unsigned int * const kingAttackersCount,unsigned int * const kingAttackersWeight,unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns) const
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
			(*pkingAttackersWeight) += _eParm.KingAttackWeights[ pieceType -2 ];
			bitMap adjacent = attack & Movegen::attackFrom<whiteKing>(enemyKingSquare);
			if(adjacent)
			{
				( *pkingAdjacentZoneAttacksCount ) += bitCnt(adjacent);
			}
		}
		attackedSquares[piece] |= attack;


		bitMap defendedPieces = attack & ourPieces & ~ourPawns;
		// piece coordination
		res += (int)bitCnt( defendedPieces ) * _eParm.pieceCoordination[ pieceType ];


		//unsigned int mobility = (bitCnt(attack&~(threatenSquares|ourPieces))+ bitCnt(attack&~(ourPieces)))/2;
		unsigned int mobility = bitCnt( attack & ~(threatenSquares | ourPieces));

		res += _eParm.mobilityBonus[ pieceType ][ mobility ];
		if(piece != whiteKnights && piece != blackKnights)
		{
			if( !(attack & ~(threatenSquares | ourPieces) ) && isSquareSet( threatenSquares, sq ) ) // zero mobility && attacked by pawn
			{
				res -= ( getPieceValue(piece) / 4 );
			}
		}
		/////////////////////////////////////////
		// center control
		/////////////////////////////////////////
		if(attack & centerBitmap)
		{
			res += (int)bitCnt(attack & centerBitmap) * _eParm.piecesCenterControl[ pieceType ];
		}
		if(attack & bigCenterBitmap)
		{
			res += (int)bitCnt(attack & bigCenterBitmap) * _eParm.piecesBigCenterControl[ pieceType ];
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
				res += _eParm.queenOn7Bonus;
			}
			//--------------------------------
			// donna su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank > RANK5 && (rankMask(sq) & enemyPawns))
			{
				res += _eParm.queenOnPawns;
			}
			break;
		}
		case whiteRooks:
		case blackRooks:
		{
			bitMap enemyBackRank = isBlackPiece(piece) ? rankMask(A1) : rankMask(A8);
			//--------------------------------
			// torre in 7a con re in 8a
			//--------------------------------
			if(relativeRank == RANK7 && (enemyKing & enemyBackRank) )
			{
				res += _eParm.rookOn7Bonus;
			}
			//--------------------------------
			// torre su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank > RANK5 && (rankMask(sq) & theirPawns))
			{
				res += _eParm.rookOnPawns;
			}
			//--------------------------------
			// torre su colonna aperta/semiaperta
			//--------------------------------
			if( !(fileMask(sq) & ourPawns) )
			{
				if( !(fileMask(sq) & theirPawns) )
				{
					res += _eParm.rookOnOpen;
				}else
				{
					res += _eParm.rookOnSemi;
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

					res -= _eParm.rookTrapped*(int)(3-mobility);
					const state & st = getActualState();
					if( isBlackPiece(piece) )
					{
						if( !st.hasCastleRight( bCastleOO | bCastleOOO ) )
						{
							res -= _eParm.rookTrappedKingWithoutCastling * (int)( 3 - mobility );
						}

					}
					else
					{
						if( !st.hasCastleRight( wCastleOO | wCastleOOO ) )
						{
							res -= _eParm.rookTrappedKingWithoutCastling * (int)( 3 - mobility ) ;
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
				res += _eParm.bishopOnOutpost;
				if( isSquareSet( supportedSquares, sq ) )
				{
					res += _eParm.bishopOnOutpostSupported;
				}
				if( isSquareSet( enemyHoles, sq ) )
				{
					res += _eParm.bishopOnHole;
				}

			}
			// alfiere cattivo
			{
				Color color = getSquareColor(sq);
				bitMap blockingPawns = ourPieces & blockedPawns & getColorBitmap(color);
				if( moreThanOneBit(blockingPawns) )
				{
					res -= (int)bitCnt(blockingPawns) * _eParm.badBishop;
				}
			}

			break;
		case whiteKnights:
		case blackKnights:
			if( isSquareSet( enemyWeakSquares, sq ) )
			{
				res += _eParm.knightOnOutpost * ( 5 - std::abs( (int)relativeRank - 5 ));
				if( isSquareSet( supportedSquares, sq ) )
				{
					res += _eParm.knightOnOutpostSupported;
				}
				if( isSquareSet( enemyHoles, sq ) )
				{
					res += _eParm.knightOnHole;
				}
			}

			{
				bitMap wpa = attack & (weakPawns) & theirPieces;
				if(wpa)
				{
					res += (int)bitCnt(wpa) * _eParm.KnightAttackingWeakPawn;
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
Score Position::_evalShieldStorm(tSquare ksq) const
{
	if( getFileOf(ksq) == FILEA )
	{
		++ksq;
	}
	if( getFileOf(ksq) == FILEH )
	{
		--ksq;
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
		ks = bitCnt(pawnShield) * _eParm.kingShieldBonus[0];
	}
	if(pawnFarShield)
	{
		ks += bitCnt(pawnFarShield) * _eParm.kingFarShieldBonus[0];
	}
	while(pawnStorm)
	{
		tSquare p = iterateBit(pawnStorm);
		ks -= std::max(0, 3 - (int)distance(p,ksq) ) * _eParm.kingStormBonus[0];
	}
	return ks;
}

template<Color c>
simdScore Position::_evalPassedPawn(bitMap pp, bitMap* attackedSquares) const
{
	tSquare kingSquare = c ? getSquareOfThePiece( blackKing ) : getSquareOfThePiece( whiteKing );
	tSquare enemyKingSquare = c ?getSquareOfThePiece( whiteKing ) : getSquareOfThePiece( blackKing );
	bitboardIndex ourPieces = c ? blackPieces : whitePieces;
	bitboardIndex enemyPieces = c ? whitePieces : blackPieces;
	bitboardIndex ourRooks = c ? blackRooks : whiteRooks;
	bitboardIndex enemyRooks = c ? whiteRooks : blackRooks;
	bitboardIndex ourPawns = c ? blackPawns : whitePawns;
	const state st = getActualState();

	simdScore score = {0,0,0,0};
	while(pp)
	{
		
		simdScore passedPawnsBonus;
		tSquare ppSq = iterateBit(pp);
		unsigned int relativeRank = getRelativeRankOf( ppSq, c );
		
		int r = relativeRank - 1;
		int rr =  r * ( r - 1 );
		int rrr =  r * r * r;
		

		passedPawnsBonus = _eParm.passedPawnBonus * rrr;
		
		bitMap forwardSquares = c ? SQUARES_IN_FRONT_OF[black][ppSq] : SQUARES_IN_FRONT_OF[white][ppSq];
		bitMap unsafeSquares = forwardSquares & (attackedSquares[enemyPieces] | getBitmap(enemyPieces) );
		passedPawnsBonus -= _eParm.passedPawnUnsafeSquares * (int)bitCnt(unsafeSquares);
		
		//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
		if(rr)
		{
			tSquare blockingSquare = ppSq + pawnPush(c);

			// bonus for king proximity to blocking square
			passedPawnsBonus += _eParm.enemyKingNearPassedPawn * int( distance( blockingSquare, enemyKingSquare ) * rr );
			passedPawnsBonus -= _eParm.ownKingNearPassedPawn * int( distance( blockingSquare, kingSquare ) * rr );
			//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
			if( getPieceAt(blockingSquare) == empty )
			{
				
				bitMap backWardSquares = c ? SQUARES_IN_FRONT_OF[white][ppSq] : SQUARES_IN_FRONT_OF[black][ppSq];

				bitMap defendedSquares = forwardSquares & attackedSquares[ ourPieces ];

			
				if ( isSquareSet( unsafeSquares, blockingSquare ) )
				{
					passedPawnsBonus -= _eParm.passedPawnBlockedSquares * rr;
				}

				//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
				if(defendedSquares)
				{
					passedPawnsBonus += _eParm.passedPawnDefendedSquares * rr * (int)bitCnt( defendedSquares );
					if( isSquareSet( defendedSquares, blockingSquare ) )
					{
						passedPawnsBonus += _eParm.passedPawnDefendedBlockingSquare * rr;
					}
				}
				//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
				if(backWardSquares & getBitmap( ourRooks ))
				{
					passedPawnsBonus += _eParm.rookBehindPassedPawn * rr;
				}
				if(backWardSquares & getBitmap( enemyRooks ))
				{
					passedPawnsBonus -= _eParm.EnemyRookBehindPassedPawn * rr;
				}
				//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;
			}
		}

		if( isLateralFile( getFileOf( ppSq ) ) )
		{
			passedPawnsBonus -= _eParm.passedPawnFileAHPenalty;
		}

		bitMap supportingPawns = getBitmap( ourPawns ) & ISOLATED_PAWN[ ppSq ];
		if( supportingPawns & rankMask(ppSq) )
		{
			passedPawnsBonus+=_eParm.passedPawnSupportedBonus*rr;
		}
		if( supportingPawns & rankMask( ppSq - pawnPush(c) ) )
		{
			passedPawnsBonus += _eParm.passedPawnSupportedBonus * ( rr / 2 );
		}
		//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;

		if( st.getNonPawnValue()[ c ? 0 : 2 ] == 0 )
		{
			tSquare promotionSquare = getPromotionSquareOf( ppSq, c );
			if( std::min( 5, (int)(7- relativeRank)) <  std::max((int)distance( enemyKingSquare, promotionSquare ) - ( (c ? isBlackTurn() : isWhiteTurn() ) ? 0 : 1 ), 0) )
			{
				passedPawnsBonus += _eParm.unstoppablePassed * rr;
			}
		}
		//std::cout<<passedPawnsBonus[0]<<" "<<passedPawnsBonus[1]<<std::endl;


		score += passedPawnsBonus;

	}
	return score;
}


template<Color c> simdScore Position::_evalKingSafety(Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares) const
{
	simdScore res = {0,0,0,0};
	bitMap AttackingPieces = c ? getBitmap(whitePieces) : getBitmap(blackPieces);
	bitMap * DefendedSquaresBy = c ? &attackedSquares[blacks] : &attackedSquares[whites];
	bitMap * AttackedSquaresBy = c ? &attackedSquares[whites] : &attackedSquares[blacks];
	tSquare kingSquare = c ? getSquareOfThePiece(blackKing) : getSquareOfThePiece(whiteKing);

	if( kingAttackersCount > ( getPieceCount( c? whiteQueens: blackQueens ) > 0 ? 0 : 1 ) )// se il re e' attaccato
	{

		bitMap undefendedSquares = AttackedSquaresBy[Pieces] & DefendedSquaresBy[King];
		undefendedSquares &=
			~( DefendedSquaresBy[Pawns]
			| DefendedSquaresBy[Knights]
			| DefendedSquaresBy[Bishops]
			| DefendedSquaresBy[Rooks]
			/*| DefendedSquaresBy[Queens]*/);
		bitMap undefendedSquares2 = ~ DefendedSquaresBy[Pieces];
		
		signed int attackUnits = kingAttackersCount * kingAttackersWeight;
		attackUnits += kingAdjacentZoneAttacksCount * _eParm.kingSafetyPars1[0];
		attackUnits += bitCnt( undefendedSquares ) * _eParm.kingSafetyPars1[1];
		attackUnits += KingExposed[ c? 63 - kingSquare : kingSquare ] * _eParm.kingSafetyPars1[2];
		attackUnits -= (getPieceCount(c? whiteQueens: blackQueens)==0) * _eParm.kingSafetyPars1[3];

		attackUnits -= 10 * kingSafety / _eParm.kingStormBonus[1] ;
		attackUnits += _eParm.kingStormBonus[2] ;



		// long distance check
		bitMap rMap = Movegen::attackFrom<whiteRooks>( kingSquare, getOccupationBitmap() );
		bitMap bMap = Movegen::attackFrom<whiteBishops>( kingSquare, getOccupationBitmap() );

		if(  (rMap | bMap) & AttackedSquaresBy[Queens] &  ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += _eParm.kingSafetyPars2[0];
		}
		if(  rMap & AttackedSquaresBy[Rooks] &  ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += _eParm.kingSafetyPars2[1];
		}
		if(  bMap & AttackedSquaresBy[Bishops] &  ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += _eParm.kingSafetyPars2[2];
		}
		if( Movegen::attackFrom<whiteKnights>( kingSquare ) & ( AttackedSquaresBy[Knights] ) & ~AttackingPieces & undefendedSquares2 )
		{
			attackUnits += _eParm.kingSafetyPars2[3];
		}

		attackUnits = std::max( 0, attackUnits );
		simdScore ks = {attackUnits * attackUnits / _eParm.kingSafetyBonus[0], 10 * attackUnits / _eParm.kingSafetyBonus[1], 0, 0 };
		res = -ks;
	}
	return res;
}


simdScore Position::_calcPawnValues(bitMap& weakPawns, bitMap& passedPawns, bitMap * const attackedSquares , bitMap * const weakSquares, bitMap * const holes) const {
	simdScore pawnResult = simdScore{0,0,0,0};
	bitMap pawns = getBitmap(whitePawns);

	while(pawns)
	{
		tSquare sq = iterateBit(pawns);
		pawnResult += _evalPawn<white>(sq, weakPawns, passedPawns);
	}

	pawns = getBitmap(blackPawns);

	while(pawns)
	{
		tSquare sq = iterateBit(pawns);
		pawnResult -= _evalPawn<black>(sq, weakPawns, passedPawns);
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
	pawnResult -= ( (int)bitCnt( holes[white] ) - (int)bitCnt( holes[black] ) ) * _eParm.holesPenalty;
	return pawnResult;
}

/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
template<bool trace>
Score Position::eval(void) const
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
	unsigned int kingAttackersCount[2] = {0};
	unsigned int kingAttackersWeight[2] = {0};
	unsigned int kingAdjacentZoneAttacksCount[2] = {0};

	//-----------------------------------------------------
	//	material evalutation
	//-----------------------------------------------------


	const materialStruct* const materialData = _getMaterialData();
	if( materialData )
	{
		bool (Position::*pointer)(Score &) const = materialData->pointer;
		switch(materialData->type)
		{
			case materialStruct::type::exact:
				return isBlackTurn() ? -materialData->val : materialData->val;
				break;
			case materialStruct::type::multiplicativeFunction:
			{
				Score r = 0;
				if( (this->*pointer)(r))
				{
					mulCoeff = r;
				}
				break;
			}
			case materialStruct::type::exactFunction:
			{
				Score r = 0;
				if( (this->*pointer)(r))
				{
					return isBlackTurn() ? -r : r;
				}
				break;
			}
			case materialStruct::type::saturationH:
				highSat = materialData->val;
				break;
			case materialStruct::type::saturationL:
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
			_evalKxvsK(r);
			return isBlackTurn() ? -r : r;
		}
	}
	
	simdScore res = st.getMaterialValue();
    
    if(std::abs(res[1]) < 20000 && _nnue && _nnue->loaded()) {
        return _nnue->eval(*this);
    }
        
    /*****************
    // populate king ring variables
    ******************/
	kingRing[white] = 0;
	if( st.getNonPawnValue()[ 2 ] >= getPieceValue(Knights)[0] + getPieceValue(Rooks)[0] )
	{
		tSquare k = getSquareOfThePiece(whiteKing);
		if( getRankOf(k) == RANK1 )
		{
			k += north;
		}
		if( getFileOf(k) == FILEA )
		{
			k += est;
		}
		if( getFileOf(k) == FILEH )
		{
			k += ovest;
		}
		kingRing[white] = Movegen::attackFrom<whiteKing>(k) | bitSet( k );
	}

	kingRing[black] = 0;
	if( st.getNonPawnValue()[ 0 ] >= getPieceValue(Knights)[0] + getPieceValue(Rooks)[0] )
	{
		tSquare k = getSquareOfThePiece(blackKing);
		if( getRankOf(k) == RANK8 )
		{
			k += sud;
		}
		if( getFileOf(k) == FILEA )
		{
			k += est;
		}
		if( getFileOf(k) == FILEH )
		{
			k += ovest;
		}
		kingRing[black] = Movegen::attackFrom<blackKing>(k) | bitSet( k );
	}


	//---------------------------------------------
	//	tempo
	//---------------------------------------------
	res += isBlackTurn() ? -_eParm.tempo : _eParm.tempo;

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
			res += _eParm.bishopPair;
		}
	}

	if( getPieceCount(blackBishops) >= 2 )
	{
		if( (getBitmap(blackBishops) & getColorBitmap(white)) && (getBitmap(blackBishops) & getColorBitmap(black) ) )
		{
			res -= _eParm.bishopPair;
		}
	}
	if( getPieceCount(blackPawns) + getPieceCount(whitePawns) == 0 )
	{
		if((int)getPieceCount(whiteQueens) - (int)getPieceCount(blackQueens) == 1
				&& (int)getPieceCount(blackRooks) - (int)getPieceCount(whiteRooks) == 1
				&& (int)getPieceCount(blackBishops) + (int)getPieceCount(blackKnights) - (int)getPieceCount(whiteBishops) - (int)getPieceCount(whiteKnights) == 2)
		{
			res += _eParm.queenVsRook2MinorsImbalance;

		}
		else if((int)getPieceCount(whiteQueens) - (int)getPieceCount(blackQueens) == -1
				&& (int)getPieceCount(blackRooks) - (int)getPieceCount(whiteRooks) == -1
				&& (int)getPieceCount(blackBishops) + (int)getPieceCount(blackKnights) - (int)getPieceCount(whiteBishops) -(int)getPieceCount(whiteKnights) == -2)
		{
			res -= _eParm.queenVsRook2MinorsImbalance;

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
	if(const HashKey& pawnKey = getPawnKey(); _pawnHashTable) {
		
		if (simdScore tableScore; _pawnHashTable->getValues(pawnKey, tableScore, weakPawns, passedPawns, attackedSquares, weakSquares, holes)) {
			res += tableScore;
		} else {
			simdScore pawnResult = _calcPawnValues(weakPawns, passedPawns, attackedSquares, weakSquares, holes);
			
			_pawnHashTable->insert(pawnKey, pawnResult, weakPawns, passedPawns, attackedSquares, weakSquares, holes);			
			res += pawnResult;

		}
	} else {
		res += _calcPawnValues(weakPawns, passedPawns, attackedSquares, weakSquares, holes);
	}
	
	//---------------------------------------------
	// center control
	//---------------------------------------------

	if( attackedSquares[whitePawns] & centerBitmap )
	{
		res += (int)bitCnt( attackedSquares[whitePawns] & centerBitmap ) * _eParm.pawnCenterControl;
	}
	if( attackedSquares[whitePawns] & bigCenterBitmap )
	{
		res += (int)bitCnt( attackedSquares[whitePawns] & bigCenterBitmap )* _eParm.pawnBigCenterControl;
	}

	if( attackedSquares[blackPawns] & centerBitmap )
	{
		res -= (int)bitCnt( attackedSquares[blackPawns] & centerBitmap ) * _eParm.pawnCenterControl;
	}
	if( attackedSquares[blackPawns] & bigCenterBitmap )
	{
		res -= (int)bitCnt( attackedSquares[blackPawns] & bigCenterBitmap ) * _eParm.pawnBigCenterControl;
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
	wScore = _evalPieces<whiteKnights>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = _evalPieces<blackKnights>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
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

	wScore = _evalPieces<whiteBishops>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = _evalPieces<blackBishops>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
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

	wScore = _evalPieces<whiteRooks>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = _evalPieces<blackRooks>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
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

	wScore = _evalPieces<whiteQueens>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
	bScore = _evalPieces<blackQueens>(weakSquares, attackedSquares, holes, blockedPawns, kingRing, kingAttackersCount, kingAttackersWeight, kingAdjacentZoneAttacksCount, weakPawns );
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


	wScore = _evalPassedPawn<white>(passedPawns & getBitmap( whitePawns ), attackedSquares);
	bScore = _evalPassedPawn<black>(passedPawns & getBitmap( blackPawns ), attackedSquares);
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

	res += ( (int)bitCnt(spacew) - (int)bitCnt(spaceb) ) * _eParm.spaceBonus;

	if(trace)
	{
		wScore = (int)bitCnt(spacew) * _eParm.spaceBonus;
		bScore = (int)bitCnt(spaceb) * _eParm.spaceBonus;
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
		wScore -= _eParm.attackedByPawnPenalty[ getPieceTypeAt(attacked) ];
	}

	// todo fare un weak piece migliore:qualsiasi pezzo attaccato riceve un malus dipendente dal suo pi� debole attaccante e dal suo valore.
	// volendo anche da quale pezzo � difeso
	bitMap undefendedMinors =  (getBitmap(whiteKnights) | getBitmap(whiteBishops))  & ~attackedSquares[whitePieces];
	if (undefendedMinors)
	{
		wScore -= _eParm.undefendedMinorPenalty;
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
				wScore -= _eParm.weakPiecePenalty[ attackedPieceType ][ getPieceType( attackingPiece ) ];
				break;
			}
		}

	}
	weakPieces =   getBitmap(whitePawns)
		& ~attackedSquares[whitePieces]
		& attackedSquares[blackKing];

	if(weakPieces)
	{
		wScore -= _eParm.weakPawnAttackedByKing;
	}

	pawnAttackedPieces = getBitmap( blackPieces ) & attackedSquares[ whitePawns ];
	while(pawnAttackedPieces)
	{
		tSquare attacked = iterateBit( pawnAttackedPieces );
		bScore -= _eParm.attackedByPawnPenalty[ getPieceTypeAt(attacked) ];
	}

	undefendedMinors =  (getBitmap(blackKnights) | getBitmap(blackBishops))  & ~attackedSquares[blackPieces];
	if (undefendedMinors)
	{
		bScore -= _eParm.undefendedMinorPenalty;
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
				bScore -= _eParm.weakPiecePenalty[ attackedPieceType ][ getPieceType(attackingPiece) ];
				break;
			}
		}
	}

	weakPieces =   getBitmap(blackPawns)
		& ~attackedSquares[blackPieces]
		& attackedSquares[whiteKing];

	if(weakPieces)
	{
		bScore -= _eParm.weakPawnAttackedByKing;
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

	kingSafety[white] = _evalShieldStorm<white>(getSquareOfThePiece(whiteKing));

	if( st.hasCastleRight( wCastleOO )
		&& !(attackedSquares[blackPieces] & getCastleKingPath(wCastleOO))
		&& !moreThanOneBit(_CastlePathOccupancyBitmap(wCastleOO))
		)
	{
		kingSafety[white] = std::max( _evalShieldStorm<white>(G1), kingSafety[white]);
	}

	if( st.hasCastleRight( wCastleOOO )
		&& !(attackedSquares[blackPieces] & getCastleKingPath(wCastleOOO))
		&& !moreThanOneBit(_CastlePathOccupancyBitmap(wCastleOOO))
		)
	{
		kingSafety[white] = std::max( _evalShieldStorm<white>(C1), kingSafety[white]);
	}
	if(trace)
	{
		wScore = simdScore{ kingSafety[white], 0, 0, 0};
	}

	kingSafety[black] = _evalShieldStorm<black>(getSquareOfThePiece(blackKing));

	if( st.hasCastleRight( bCastleOO )
		&& !(attackedSquares[whitePieces] & getCastleKingPath(bCastleOO))
		&& !moreThanOneBit(_CastlePathOccupancyBitmap(bCastleOO))
		)
	{
		kingSafety[black] = std::max( _evalShieldStorm<black>(G8), kingSafety[black]);
	}

	if( st.hasCastleRight( bCastleOOO )
		&& !(attackedSquares[whitePieces] & getCastleKingPath(bCastleOOO))
		&& !moreThanOneBit(_CastlePathOccupancyBitmap(bCastleOOO))
		)
	{
		kingSafety[black] = std::max(_evalShieldStorm<black>(C8), kingSafety[black]);
	}
	if(trace)
	{
		bScore = simdScore{ kingSafety[black], 0, 0, 0};
	}

	res+=simdScore{kingSafety[white]-kingSafety[black],0,0,0};




	simdScore kingSaf = _evalKingSafety<white>(kingSafety[white], kingAttackersCount[black], kingAdjacentZoneAttacksCount[black], kingAttackersWeight[black], attackedSquares);

	res += kingSaf;
	if(trace)
	{
		wScore += kingSaf;
	}

	kingSaf = _evalKingSafety<black>(kingSafety[black], kingAttackersCount[white], kingAdjacentZoneAttacksCount[white], kingAttackersWeight[white], attackedSquares);

	res-= kingSaf;
	if(trace)
	{
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
	
	if (mulCoeff == 256) {
		mulCoeff = std::min(160 + (isOppositeBishops() ? 4 : 14) * (getPieceCount(whitePawns)+ getPieceCount(blackPawns)), 256u);
	}
	
	if (trace) {
		sync_cout << "mulCoeff:" <<  mulCoeff / 2.56 << sync_endl;
	}

	//--------------------------------------
	//	finalizing
	//--------------------------------------
	signed int gamePhase = getGamePhase( st );
	// mulCoeff will multiplicate only endgame
	signed long long r = (((signed long long)res[0]) * (65536 - gamePhase)) + (((signed long long)res[1]) * gamePhase * mulCoeff / 256);

	Score score = (Score)( (r) / 65536 );

	// final value saturation
	score = std::min(highSat,score);
	score = std::max(lowSat,score);

	return isBlackTurn() ? -score : score;

}

template Score Position::eval<false>(void) const;
template Score Position::eval<true>(void) const;
