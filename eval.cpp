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
#include "move.h"
#include "bitops.h"
#include "movegen.h"


enum color{
	white=0,
	black=1
};

const simdScore mobilityBonus[][32] = {
		{}, {},
		{ simdScore(-1352,-2686,0,0), simdScore( -901,-1747,0,0), simdScore(-563, -939,0,0), simdScore(-225, -134,0,0), simdScore( 112,  670,0,0), simdScore( 450, 1475,0,0), // Queens
				simdScore(  788, 2284,0,0), simdScore(1127, 3089,0,0), simdScore(1468, 3894,0,0), simdScore(1806, 4568,0,0), simdScore(2031, 5104,0,0), simdScore(2257, 5373,0,0),
				simdScore( 2482, 5507,0,0), simdScore( 2595,5507,0,0), simdScore(2707, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0),
				simdScore( 2820, 5507,0,0), simdScore( 2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0),
				simdScore( 2820, 5507,0,0), simdScore( 2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0), simdScore(2820, 5507,0,0),
				simdScore( 2820, 5507,0,0), simdScore( 2820, 5507,0,0) },
		{ simdScore(-1918,-4434,0,0), simdScore(-1239,-2150,0,0), simdScore(-563,  0,0,0), simdScore( 112, 2150,0,0), simdScore( 788, 4300,0,0), simdScore(1465, 6450,0,0), // Rooks
				simdScore( 2031, 8600,0,0), simdScore(2482, 10750,0,0), simdScore(2933, 12900,0,0), simdScore(3271,14644,0,0), simdScore(3496,15452,0,0), simdScore(3725,15989,0,0),
				simdScore( 3835,16391,0,0), simdScore( 4063,16529,0,0), simdScore(4176,16659,0,0), simdScore(4288,16659,0,0) },
		{ simdScore(-2483,-3628,0,0), simdScore( -903,-1746,0,0), simdScore( 677,  134,0,0), simdScore(2257, 2015,0,0), simdScore(3838, 3896,0,0), simdScore(5418, 5778,0,0), // Bishops
				simdScore( 6773, 7390,0,0), simdScore( 7676, 8465,0,0), simdScore(8353, 9137,0,0), simdScore(8692,9675,0,0), simdScore(9031, 10078,0,0), simdScore(9257, 10346,0,0),
				simdScore( 9482, 10615,0,0), simdScore( 9708, 10884,0,0), simdScore(9821, 11018,0,0), simdScore(9821, 11018,0,0) },
		{ simdScore(-3951,-30,0,0), simdScore(-2843,-20,0,0), simdScore(-1016,-10,0,0), simdScore( 338,  0,0,0), simdScore(1693, 10,0,0), simdScore(3048, 20,0,0), // Knights
				simdScore( 4176, 28,0,0), simdScore( 4741, 31,0,0), simdScore(4967, 33,0,0) }
};

const int KingExposed[] = {
     2,  0,  2,  5,  5,  2,  0,  2,
     2,  2,  4,  8,  8,  4,  2,  2,
     7, 10, 12, 12, 12, 12, 10,  7,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15
  };



//------------------------------------------------
//	PAWN Bonus/Penalties
//------------------------------------------------
simdScore isolatedPawnPenalty=simdScore(1000,1100,0,0);
simdScore doubledPawnPenalty=simdScore(500,500,0,0);
simdScore backwardPawnPenalty=simdScore(600,600,0,0);
simdScore chainedPawnBonus=simdScore(1200,1100,0,0);
simdScore passedPawnFileAHPenalty = simdScore(0,2000,0,0);
simdScore passedPawnSupportedBonus = simdScore(0,1000,0,0);
simdScore candidateBonus = simdScore(1000,100,0,0);


simdScore rookOn7Bonus=simdScore(5700,3600,0,0);
simdScore rookOnPawns=simdScore(1000,3000,0,0);
simdScore queenOn7Bonus=simdScore(200,1600,0,0);
simdScore queenOnPawns=simdScore(500,1000,0,0);
simdScore rookOnOpen=simdScore(2000,500,0,0);
simdScore rookOnSemi=simdScore(1000,500,0,0);

simdScore knightOnOutpost= simdScore(100,120,0,0);
simdScore knightOnOutpostSupported= simdScore(210,190,0,0);
simdScore knightOnHole= simdScore(310,390,0,0);

simdScore bishopOnOutpost= simdScore(80,110,0,0);
simdScore bishopOnOutpostSupported= simdScore(1900,170,0,0);
simdScore bishopOnHole= simdScore(290,370,0,0);

simdScore tempo= simdScore(1540,421,0,0);
//------------------------------------------------
//king safety
//------------------------------------------------
const unsigned int KingAttackWeights[] = { 0, 0, 5, 3, 2, 2 };
simdScore kingShieldBonus= simdScore(1600,800,0,0);
simdScore kingFarShieldBonus= simdScore(1000,400,0,0);
//------------------------------------------------
template<color c>
simdScore evalPawn(const Position & p,tSquare sq, bitMap & weakPawns, bitMap & passedPawns){

	simdScore res=0;

	bool passed, isolated, doubled, opposed, chain, backward;
	const bitMap ourPawns =c?p.bitBoard[Position::blackPawns]:p.bitBoard[Position::whitePawns];
	const bitMap theirPawns =c?p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
	bitMap b;
	const int relativeRank =c?7-RANKS[sq] :RANKS[sq];
	const int file =FILES[sq];

	// Our rank plus previous one. Used for chain detection
    b = RANKMASK[sq] | RANKMASK[sq-pawnPush(c)];
    // Flag the pawn as passed, isolated, doubled or member of a pawn
    // chain (but not the backward one).
    chain    = (ourPawns   & ISOLATED_PAWN[sq] & b);
	isolated = !(ourPawns   & ISOLATED_PAWN[sq]);
    doubled  = (ourPawns   & SQUARES_IN_FRONT_OF[c][sq])!=0;
    opposed  = (theirPawns & SQUARES_IN_FRONT_OF[c][sq])!=0;
    passed   = (theirPawns & PASSED_PAWN[c][sq])==0;

	backward=false;
	if(
		!(passed | isolated | chain) &&
		!(ourPawns & PASSED_PAWN[1-c][sq+pawnPush(c)] & ISOLATED_PAWN[sq]))// non ci sono nostri pedoni dietro a noi
	{
		b = RANKMASK[sq+pawnPush(c)]& ISOLATED_PAWN[sq];
		while (!(b & (ourPawns | theirPawns))){
			if(!c){
				b <<= 8;
			}
			else{
				b >>= 8;
			}

		}
		backward = ((b | ((!c)?(b << 8):(b >> 8))) & theirPawns)!=0;



	}

	if (isolated){
		res -= isolatedPawnPenalty;
		weakPawns|=BITSET[sq];

	}

    if (doubled){
    	res-= doubledPawnPenalty;
	//	weakPawns|=BITSET[sq];
	}

    if (backward){
    	res-= backwardPawnPenalty;
		weakPawns|=BITSET[sq];
	}

    if (chain){
    	res+= chainedPawnBonus;
	}


	//passed pawn
	if(passed &&!doubled){
		int r=relativeRank-1;
		int rr= r*(r-1);

		res+=simdScore(600*rr,1100*(rr+r+1),0,0);

		passedPawns|=BITSET[sq];

		if(file==0 || file==7){
			res -=passedPawnFileAHPenalty;
		}
		if(chain){
			res+=passedPawnSupportedBonus*r;
		}
	}

	if ( !passed && !isolated && !doubled && !opposed && bitCnt( PASSED_PAWN[c][sq] & theirPawns ) < bitCnt(PASSED_PAWN[c][sq-pawnPush(c)] & ourPawns ))
	{
		res+=candidateBonus*(relativeRank-1);
	}
	return res;
}

template<Position::bitboardIndex piece>
simdScore evalPieces(const Position & p, const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes, bitMap * const kingRing,unsigned int * const kingAttackersCount,unsigned int * const kingAttackersWeight,unsigned int * const kingAdjacentZoneAttacksCount){
	simdScore res=0;
	bitMap tempPieces=p.bitBoard[piece];
	bitMap enemyKing =(piece>Position::separationBitmap)? p.bitBoard[Position::whiteKing]:p.bitBoard[Position::blackKing];
	tSquare enemyKingSquare =(piece>Position::separationBitmap)? p.pieceList[Position::whiteKing][0]:p.pieceList[Position::blackKing][0];
	bitMap enemyKingRing =(piece>Position::separationBitmap)? kingRing[white]:kingRing[black];
	unsigned int * pKingAttackersCount=(piece>Position::separationBitmap)?&kingAttackersCount[black]:&kingAttackersCount[white];
	unsigned int * pkingAttackersWeight=(piece>Position::separationBitmap)?&kingAttackersWeight[black]:&kingAttackersWeight[white];
	unsigned int * pkingAdjacentZoneAttacksCount=(piece>Position::separationBitmap)?&kingAdjacentZoneAttacksCount[black]:&kingAdjacentZoneAttacksCount[white];
	const bitMap & enemyWeakSquares=(piece>Position::separationBitmap)? weakSquares[white]:weakSquares[black];
	const bitMap & enemyHoles=(piece>Position::separationBitmap)? holes[white]:holes[black];
	const bitMap & supportedSquares=(piece>Position::separationBitmap)? attackedSquares[Position::blackPawns]:attackedSquares[Position::whitePawns];
	const bitMap & threatenSquares=(piece>Position::separationBitmap)? attackedSquares[Position::whitePawns]:attackedSquares[Position::blackPawns];
	while(tempPieces){
		tSquare sq=firstOne(tempPieces);
		tempPieces&= tempPieces-1;
		unsigned int relativeRank =(piece>Position::separationBitmap)? 7-RANKS[sq]:RANKS[sq];

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti
		// todo fare una tabella per ogni pezzo



		bitMap attack;


		switch(piece){

		case Position::whiteRooks:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::whiteRooks]^p.bitBoard[Position::whiteQueens]);
			break;
		case Position::blackRooks:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::blackRooks]^p.bitBoard[Position::blackQueens]);
			break;
		case Position::whiteBishops:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::whiteQueens]);
			break;
		case Position::blackBishops:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]^p.bitBoard[Position::blackQueens]);
			break;
		case Position::whiteQueens:
		case Position::blackQueens:
		case Position::whiteKnights:
		case Position::blackKnights:
			attack = Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]);
		default:
			break;
		}

		if(attack & enemyKingRing){
			(*pKingAttackersCount)++;
			(*pkingAttackersWeight)+=KingAttackWeights[piece%Position::separationBitmap];
			bitMap adjacent=attack& Movegen::attackFromKing(enemyKingSquare);
			if(adjacent)
			{
				(*pkingAdjacentZoneAttacksCount)+=bitCnt(adjacent);
			}
		}
		attackedSquares[piece]|=attack;


		unsigned int mobility= bitCnt(attack&~threatenSquares);
		res+=mobilityBonus[piece%Position::separationBitmap][mobility];

		//todo alfiere buono cattivo
		//todo controllo centro
		//todo trapped pieces
		switch(piece){
		case Position::whiteQueens:
		case Position::blackQueens:
		{
			bitMap enemyBackRank=(piece>Position::separationBitmap)? RANKMASK[A1]:RANKMASK[A8];
			bitMap enemyPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
			//--------------------------------
			// donna in 7a con re in 8a
			//--------------------------------
			if(relativeRank==6 &&(enemyKing & enemyBackRank) ){
				res+=queenOn7Bonus;
			}
			//--------------------------------
			// donna su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank>4 && (RANKMASK[sq]& enemyPawns)){
				res+=queenOnPawns;
			}
			break;
		}
		case Position::whiteRooks:
		case Position::blackRooks:
		{
			bitMap enemyBackRank=(piece>Position::separationBitmap)? RANKMASK[A1]:RANKMASK[A8];
			bitMap enemyPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
			bitMap ourPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::blackPawns]:p.bitBoard[Position::whitePawns];
			//--------------------------------
			// torre in 7a con re in 8a
			//--------------------------------
			if(relativeRank==6 &&(enemyKing & enemyBackRank) ){
				res+=rookOn7Bonus;
			}
			//--------------------------------
			// torre su traversa che contiene pedoni
			//--------------------------------
			if(relativeRank>4 && (RANKMASK[sq]& enemyPawns)){
				res+=rookOnPawns;
			}
			//--------------------------------
			// torre su colonna aperta/semiaperta
			//--------------------------------
			if(!(FILEMASK[sq]& ourPawns))
			{
				if(!(FILEMASK[sq]& enemyPawns))
				{
					res+=rookOnOpen;
				}else
				{
					res+=rookOnSemi;
				}
			}
			break;
		}
		case Position::whiteBishops:
		case Position::blackBishops:
			if(relativeRank>=4 && (enemyWeakSquares& BITSET[sq]))
			{
				res+=bishopOnOutpost;
				if(supportedSquares &BITSET[sq]){
					res += bishopOnOutpostSupported;
				}
				if(enemyHoles &BITSET[sq]){
					res += bishopOnHole;
				}

			}

			break;
		case Position::whiteKnights:
		case Position::blackKnights:
			if(relativeRank>=4 && (enemyWeakSquares& BITSET[sq]))
			{
				res+=knightOnOutpost;
				if(supportedSquares &BITSET[sq]){
					res += knightOnOutpostSupported;
				}
				if(enemyHoles &BITSET[sq]){
					res += knightOnHole;
				}

			}
			break;
		default:
			break;
		}
	}
	return res;
}


/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
Score Position::eval(void) const {

	bitMap attackedSquares[lastBitboard]={0};
	bitMap weakSquares[2]={0};
	bitMap holes[2]={0};
	bitMap kingRing[2]={0};
	bitMap kingShield[2]={0};
	bitMap kingFarShield[2]={0};
	unsigned int kingAttackersCount[2]={0};
	unsigned int kingAttackersWeight[2]={0};
	unsigned int kingAdjacentZoneAttacksCount[2]={0};

	tSquare k=pieceList[whiteKing][0];
	kingRing[white]=Movegen::attackFromKing(k);
	kingShield[white]=kingRing[white];
	if(RANKS[k]<7){kingRing[white]|=Movegen::attackFromKing(tSquare(k+8));}
	kingFarShield[white]=kingRing[white]&~(kingShield[white]|BITSET[k]);


	k=pieceList[blackKing][0];
	kingRing[black]=Movegen::attackFromKing(k);
	kingShield[black]=kingRing[black];
	if(RANKS[k]>0){kingRing[black]|=Movegen::attackFromKing(tSquare(k-8));}
	kingFarShield[black]=kingRing[black]&~(kingShield[black]|BITSET[k]);



	// todo modificare valori material value & pst
	// material + pst
	state &st =getActualState();
	simdScore res=simdScore(st.material[0],st.material[1],0,0);

	//---------------------------------------------
	//	tempo
	//---------------------------------------------
	if(st.nextMove)
	{
		res-=tempo;
	}
	else{
		res+=tempo;
	}

	//todo specialized endgame & scaling function
	//todo material imbalance
	bitMap weakPawns=0;
	bitMap passedPawns=0;

	//----------------------------------------------
	//	PAWNS EVALUTATION
	//----------------------------------------------
	//todo creare pawn attack bitmaps
	//todo king shield
	//todo passed pawn post evalutation
	//todo king near passed pawn
	//todo supporting / counter passed pawn
	bitMap pawns= bitBoard[whitePawns];

	while(pawns){
		tSquare sq=firstOne(pawns);
		res+=evalPawn<white>(*this,sq, weakPawns, passedPawns);
		pawns &= pawns-1;
	}

	pawns= bitBoard[blackPawns];

	while(pawns){
		tSquare sq=firstOne(pawns);
		res-=evalPawn<black>(*this,sq, weakPawns, passedPawns);
		pawns &= pawns-1;
	}

	//------------------------------------------
	//	weak squares
	//------------------------------------------



	bitMap temp=bitBoard[whitePawns];

	bitMap pawnAttack=(temp & ~(FILEMASK[H1]))<<9;
	pawnAttack|=(temp & ~(FILEMASK[A1]))<<7;

	attackedSquares[whitePawns]=pawnAttack;

	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;

	weakSquares[white]=~pawnAttack;


	temp=bitBoard[whitePawns]<<8;
	temp|=temp<<8;
	temp|=temp<<8;
	temp|=temp<<8;
	temp|=temp<<8;
	temp|=temp<<8;

	holes[white]= weakSquares[white]&temp;

	temp=bitBoard[blackPawns];

	pawnAttack=(temp & ~(FILEMASK[H1]))>>7;
	pawnAttack|=(temp & ~(FILEMASK[A1]))>>9;

	attackedSquares[blackPawns]=pawnAttack;


	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;

	weakSquares[black]=~pawnAttack;

	temp=bitBoard[blackPawns]>>8;
	temp|=temp>>8;
	temp|=temp>>8;
	temp|=temp>>8;
	temp|=temp>>8;
	temp|=temp>>8;

	holes[black]= weakSquares[black]&temp;





	// todo supported squares


	//-----------------------------------------
	//	pieces
	//-----------------------------------------

	res+=evalPieces<Position::whiteQueens>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=evalPieces<Position::whiteRooks>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=evalPieces<Position::whiteBishops>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res+=evalPieces<Position::whiteKnights>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);

	res-=evalPieces<Position::blackQueens>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res-=evalPieces<Position::blackRooks>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res-=evalPieces<Position::blackBishops>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);
	res-=evalPieces<Position::blackKnights>(*this,weakSquares,attackedSquares,holes,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount);

	//todo valutazione pezzi

	attackedSquares[whiteKing]=Movegen::attackFromKing(pieceList[whiteKing][0]);
	attackedSquares[blackKing]=Movegen::attackFromKing(pieceList[blackKing][0]);


	//todo attacked squares

	//todo space

	//todo counterattack??

	//todo weakpawn

	//todo unsopported pieces/weak pieces
	//todo undefended minor

	//todo king shield
	//todo king safety
	//todo scaling
	// finalizing

	//--------------------------------------
	//	king safety
	//--------------------------------------
	// todo tenere conto anche della possibilita di arrocco
	bitMap pawnShield=kingShield[white]&bitBoard[whitePawns];
	bitMap pawnFarShield=kingFarShield[white]&bitBoard[whitePawns];
	if(pawnShield){
		res+=bitCnt(pawnShield)*kingShieldBonus;
	}
	if(pawnFarShield){
		res+=bitCnt(pawnFarShield)*kingFarShieldBonus;
	}
	pawnShield=kingShield[black]&bitBoard[blackPawns];
	pawnFarShield=kingFarShield[black]&bitBoard[blackPawns];
	if(pawnShield){
		res-=bitCnt(pawnShield)*kingShieldBonus;
	}
	if(pawnFarShield){
		res-=bitCnt(pawnFarShield)*kingFarShieldBonus;
	}


	if(kingAttackersCount[white]>=2 && kingAdjacentZoneAttacksCount[white])
	{

		bitMap undefendedSquares=
			(attackedSquares[whitePawns]
			|attackedSquares[whiteKnights]
			|attackedSquares[whiteBishops]
			|attackedSquares[whiteRooks]
			|attackedSquares[whiteQueens]
			|attackedSquares[whiteKing])& attackedSquares[blackKing];
		undefendedSquares&=
			~(attackedSquares[blackPawns]
			|attackedSquares[blackKnights]
			|attackedSquares[blackBishops]
			|attackedSquares[blackRooks]
			|attackedSquares[blackQueens]);

		unsigned int attackUnits =  std::min((unsigned int)25, (kingAttackersCount[white] * kingAttackersWeight[white]) / 2)
		                     + 3 * (kingAdjacentZoneAttacksCount[white] + bitCnt(undefendedSquares))
		                     + KingExposed[64-pieceList[blackKing][0]];
		                     //- mg_value(score) / 32;
		attackUnits = std::min((unsigned int)99, std::max((unsigned int)0, attackUnits));
		attackUnits*=attackUnits;
		res+=simdScore(attackUnits,attackUnits/2,0,0);


	}

	if(kingAttackersCount[black]>=2 && kingAdjacentZoneAttacksCount[black])
	{

		bitMap undefendedSquares=
			(attackedSquares[blackPawns]
			|attackedSquares[blackKnights]
			|attackedSquares[blackBishops]
			|attackedSquares[blackRooks]
			|attackedSquares[blackQueens]
			|attackedSquares[blackKing])& attackedSquares[whiteKing];
		undefendedSquares&=
			~(attackedSquares[whitePawns]
			|attackedSquares[whiteKnights]
			|attackedSquares[whiteBishops]
			|attackedSquares[whiteRooks]
			|attackedSquares[whiteQueens]);

		unsigned int attackUnits =  std::min((unsigned int)25, (kingAttackersCount[black] * kingAttackersWeight[black]) / 2)
							 + 3 * (kingAdjacentZoneAttacksCount[black] + bitCnt(undefendedSquares))
							 + KingExposed[pieceList[whiteKing][0]];
							 //- mg_value(score) / 32;
		attackUnits = std::min((unsigned int)99, std::max((unsigned int)0, attackUnits));
		attackUnits*=attackUnits;
		res-=simdScore(attackUnits,attackUnits/2,0,0);


	}

	//--------------------------------------
	//	finalizing
	//--------------------------------------
	signed int gamePhase=getGamePhase();

	signed long long r=((signed long long)res[0])*(65536-gamePhase)+((signed long long)res[1])*gamePhase;
	Score score =(r)/65536;


	if(st.nextMove)
	{
		return -score;
	}
	else{
		return score;
	}

}
