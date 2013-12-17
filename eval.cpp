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
simdScore evalPieces(const Position & p){
	simdScore res=0;
	bitMap tempPieces=p.bitBoard[piece];

	while(tempPieces){
		tSquare sq=firstOne(tempPieces);
		tempPieces&= tempPieces-1;

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti
		unsigned int mobility= bitCnt(Movegen::attackFrom(piece,sq,p.bitBoard[Position::occupiedSquares]));
		res+=mobility*simdScore(300,250,0,0);

		//todo avamposti e possibilità di saltare su un avamposto
		//todo alfiere buono cattivo
		//todo torre in 7a con re in 8a
		//todo torre/donna su traversa con moti pedoni nemici
		//todo torre/donna su colonna aperta/semiaperta
		//todo controllo centro
		//todo trapped pieces
		switch(piece){
		case Position::whiteQueens:
			break;
		case Position::whiteRooks:
			break;
		case Position::whiteBishops:
			break;
		case Position::whiteKnights:
			break;
		case Position::blackQueens:
			break;
		case Position::blackRooks:
			break;
		case Position::blackBishops:
			break;
		case Position::blackKnights:
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

	//bitMap attackedSquares[lastBitboard]={0};

	// todo modificare valori material value & pst
	// material + pst
	state &st =getActualState();
	simdScore res=simdScore(st.material[0],st.material[1],0,0);

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


/*	bitMap pawnAttackSpawn[2]={0};

	bitMap temp=bitBoard[whitePawns];

	bitMap pawnAttack=(temp & ~(FILEMASK[H1]))<<9;
	pawnAttack|=(temp & ~(FILEMASK[A1]))<<7;

	attackedSquares[whitePawns]=pawnAttack;

	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;
	pawnAttack|=pawnAttack<<8;

	pawnAttackSpawn[white]=pawnAttack;
	display();
	displayBitmap(pawnAttackSpawn[white]);

	temp=bitBoard[whitePawns];

	pawnAttack=(temp & ~(FILEMASK[H1]))>>7;
	pawnAttack|=(temp & ~(FILEMASK[A1]))>>9;

	attackedSquares[blackPawns]=pawnAttack;

	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;
	pawnAttack|=pawnAttack>>8;

	pawnAttackSpawn[black]=pawnAttack;

*/
	//todo king attackers
	// todo supported squares


	//-----------------------------------------
	//	pieces
	//-----------------------------------------

	res+=evalPieces<Position::whiteQueens>(*this);
	res+=evalPieces<Position::whiteRooks>(*this);
	res+=evalPieces<Position::whiteBishops>(*this);
	res+=evalPieces<Position::whiteKnights>(*this);

	res-=evalPieces<Position::blackQueens>(*this);
	res-=evalPieces<Position::blackRooks>(*this);
	res-=evalPieces<Position::blackBishops>(*this);
	res-=evalPieces<Position::blackKnights>(*this);

	//todo valutazione pezzi


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
