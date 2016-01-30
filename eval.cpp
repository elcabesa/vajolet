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

#include <utility>
#include <unordered_map>
#include <map>
#include <iomanip>
#include "position.h"
#include "move.h"
#include "bitops.h"
#include "movegen.h"
#include "eval.h"


enum color{
	white=0,
	black=1
};


simdScore traceRes=simdScore(0,0,0,0);

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
//	MOBILITY BONUS
//------------------------------------------------
simdScore queenMobilityPars=simdScore(5,5,20,335);
simdScore rookMobilityPars=simdScore(5,5,215,230);
simdScore bishopMobilityPars=simdScore(4,4,283,318);
simdScore knightMobilityPars=simdScore(3,3,280,220);
simdScore mobilityBonus[Position::separationBitmap][32];


//------------------------------------------------
//	PAWN Bonus/Penalties
//------------------------------------------------
simdScore isolatedPawnPenalty=simdScore(520,2410,0,0);
simdScore isolatedPawnPenaltyOpp=simdScore(-250,-380,0,0);
simdScore doubledPawnPenalty=simdScore(290,1950,0,0);
simdScore backwardPawnPenalty=simdScore(1120,2680,0,0);
simdScore chainedPawnBonus=simdScore(540,180,0,0);
simdScore passedPawnFileAHPenalty = simdScore(-1030,720,0,0);
simdScore passedPawnSupportedBonus = simdScore(1060,-450,0,0);
simdScore candidateBonus = simdScore(110,1330,0,0);
simdScore passedPawnBonus = simdScore(460,840,0,0);
simdScore passedPawnUnsafeSquares =simdScore(-10,70,0,0);
simdScore passedPawnBlockedSquares =simdScore(150,370,0,0);
simdScore passedPawnDefendedSquares = simdScore(150,240,0,0);
simdScore passedPawnDefendedBlockingSquare = simdScore(260,170,0,0);
simdScore unstoppablePassed = simdScore(0,2000,0,0);
simdScore rookBehindPassedPawn = simdScore(0,10,0,0);
simdScore EnemyRookBehindPassedPawn = simdScore(0,10,0,0);

simdScore holesPenalty=simdScore(100,260,0,0);
simdScore pawnCenterControl=simdScore(70,120,0,0);
simdScore pawnBigCenterControl=simdScore(360,-670,0,0);


simdScore pieceCoordination=simdScore(280,160,0,0);

simdScore piecesCenterControl=simdScore(170,160,0,0);
simdScore piecesBigCenterControl=simdScore(120,50,0,0);

simdScore rookOn7Bonus=simdScore(3700,1800,0,0);
simdScore rookOnPawns=simdScore(-600,1300,0,0);
simdScore queenOn7Bonus=simdScore(-3200,7000,0,0);
simdScore queenOnPawns=simdScore(-1000,3000,0,0);
simdScore rookOnOpen=simdScore(2000,500,0,0);
simdScore rookOnSemi=simdScore(500,1100,0,0);
simdScore rookTrapped = simdScore(300,0,0,0);
simdScore rookTrappedKingWithoutCastlig = simdScore(300,0,0,0);

simdScore knightOnOutpost= simdScore(380,0,0,0);
simdScore knightOnOutpostSupported= simdScore(100,1290,0,0);
simdScore knightOnHole= simdScore(1610,1190,0,0);
simdScore KnightAttackingWeakPawn= simdScore(300,300,0,0);

simdScore bishopOnOutpost= simdScore(-1020,810,0,0);
simdScore bishopOnOutpostSupported= simdScore(3600,270,0,0);
simdScore bishopOnHole= simdScore(590,-730,0,0);
simdScore badBishop= simdScore(-200,1530,0,0);

simdScore tempo= simdScore(530,480,0,0);
simdScore bishopPair =simdScore(3260,4690,0,0);

simdScore ownKingNearPassedPawn=simdScore(0,150,0,0);
simdScore enemyKingNearPassedPawn=simdScore(10,240,0,0);

simdScore spaceBonus=simdScore(600,200,0,0);
simdScore undefendedMinorPenalty = simdScore(756,354,0,0);

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	simdScore(0,0,0,0),
	simdScore(0,0,0,0),//king
	simdScore(1000,-7800,0,0),//queen
	simdScore(2225,1627,0,0),//rook
	simdScore(3200,350,0,0),//bishop
	simdScore(2900,2750,0,0),//knight
	simdScore(0,0,0,0),//pawn
	simdScore(0,0,0,0),
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(0,0,0,0),			simdScore(0,0,0,0),		simdScore(0,0,0,0),		simdScore(0,0,0,0),		simdScore(0,0,0,0),	simdScore(0,0,0,0)},
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(0,0,0,0),			simdScore(0,0,0,0),		simdScore(0,0,0,0),		simdScore(0,0,0,0),		simdScore(0,0,0,0),	simdScore(0,0,0,0)},//king
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(-460,300,0,0),	simdScore(3812,120,0,0),simdScore(3580,600,0,0),simdScore(1420,-4400,0,0),simdScore(1400,2700,0,0),	simdScore(0,0,0,0)},//queen
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(1200,3200,0,0),	simdScore(3220,2420,0,0),simdScore(4900,3100,0,0),simdScore(3400,1700,0,0),simdScore(1000,100,0,0),	simdScore(0,0,0,0)},//rook
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(2400,4000,0,0),	simdScore(2820,1850,0,0),simdScore(400,2800,0,0),simdScore(3100,4400,0,0),simdScore(-900,4690,0,0),	simdScore(0,0,0,0)},//bishop
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(1300,400,0,0),	simdScore(3121,3147,0,0),simdScore(1723,2917,0,0),simdScore(2223,1117,0,0),simdScore(-810,554,0,0),	simdScore(0,0,0,0)},//knight
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(50,40,0,0),		simdScore(60,50,0,0),	simdScore(100,80,0,0),	simdScore(150,90,0,0),	simdScore(200,170,0,0),	simdScore(0,0,0,0)},//pawn
	{simdScore(0,0,0,0),simdScore(0,0,0,0),	simdScore(0,0,0,0),			simdScore(0,0,0,0),		simdScore(0,0,0,0),		simdScore(0,0,0,0),		simdScore(0,0,0,0),	simdScore(0,0,0,0)}
//						king				queen						rook					bishop					knight					pawn
};
//------------------------------------------------
//king safety
//------------------------------------------------
const unsigned int KingAttackWeights[] = { 0, 0, 5, 3, 2, 2 };
simdScore kingShieldBonus= 2400;
simdScore kingFarShieldBonus= 1800;
simdScore kingStormBonus= 80;
simdScore kingSafetyBonus=simdScore(93,-5,0,0);
simdScore kingSafetyScaling=simdScore(310,0,0,0);
simdScore KingSafetyMaxAttack=simdScore(93,0,0,0);
simdScore KingSafetyLinearCoefficent=simdScore(5,0,0,0);
simdScore KingSafetyMaxResult=simdScore(1000,0,0,0);
//------------------------------------------------


simdScore queenVsRook2MinorsImbalance=simdScore(20000,20000,0,0);


//---------------------------------------------
//	MATERIAL KEYS
//---------------------------------------------


struct materialStruct
{
	enum
	{
		exact,
		multiplicativeFunction,
		exactFunction,
		saturationH,
		saturationL
	} type ;
	bool (*pointer)(const Position & ,Score &);
	Score val;

};

std::unordered_map<U64, materialStruct> materialKeyMap;

bool evalKBPvsK(const Position& p, Score& res){
	color Pcolor=p.getBitmap(Position::whitePawns)?white:black;
	tSquare pawnSquare;
	tSquare bishopSquare;
	if(Pcolor==white){
		pawnSquare = p.getSquareOfThePiece(Position::whitePawns);
		int pawnFile=FILES[pawnSquare];
		if(pawnFile==0 || pawnFile==7){
			bishopSquare=p.getSquareOfThePiece(Position::whiteBishops);
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][7]]!=SQUARE_COLOR[bishopSquare]){
				tSquare kingSquare=  p.getSquareOfThePiece(Position::blackKing);
				if(RANKS[kingSquare]>=6  && abs(pawnFile-FILES[kingSquare])<=1){
					res=0;
					return true;
				}
			}
		}
	}
	else{
		pawnSquare = p.getSquareOfThePiece(Position::blackPawns);
		int pawnFile=FILES[pawnSquare];
		if(pawnFile==0 || pawnFile==7){
			bishopSquare=p.getSquareOfThePiece(Position::blackBishops);
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][0]]!=SQUARE_COLOR[bishopSquare]){
				tSquare kingSquare=  p.getSquareOfThePiece(Position::whiteKing);
				if(RANKS[kingSquare]<=1  && abs(pawnFile-FILES[kingSquare])<=1){
					res=0;
					return true;

				}
			}
		}
	}
	return false;

}

bool evalKQvsKP(const Position& p, Score& res){
	color Pcolor=p.getBitmap(Position::whitePawns)?white:black;
	tSquare pawnSquare;
	tSquare winningKingSquare;
	tSquare losingKingSquare;


	if(Pcolor==white){
		pawnSquare = p.getSquareOfThePiece(Position::whitePawns);
		winningKingSquare = p.getSquareOfThePiece(Position::blackKing);
		losingKingSquare = p.getSquareOfThePiece(Position::whiteKing);

		int pawnFile=FILES[pawnSquare];
		int pawnRank=RANKS[pawnSquare];
		res = -100*(7- SQUARE_DISTANCE[winningKingSquare][losingKingSquare]);

		if(
				pawnRank!=6
				|| SQUARE_DISTANCE[losingKingSquare][pawnSquare]!=1
				|| (pawnFile==1 || pawnFile==3 || pawnFile==4 || pawnFile==6) ){
			res -=90000;
		}
		return true;

	}
	else{
		pawnSquare = p.getSquareOfThePiece(Position::blackPawns);
		winningKingSquare = p.getSquareOfThePiece(Position::whiteKing);
		losingKingSquare = p.getSquareOfThePiece(Position::blackKing);

		int pawnFile=FILES[pawnSquare];
		int pawnRank=RANKS[pawnSquare];
		res = 100*(7- SQUARE_DISTANCE[winningKingSquare][losingKingSquare]);

		if(
				pawnRank!=1
				|| SQUARE_DISTANCE[losingKingSquare][pawnSquare]!=1
				|| (pawnFile==1 || pawnFile==3 || pawnFile==4 || pawnFile==6) ){
			res +=90000;
		}
		return true;

	}
	return false;

}


bool evalKRPvsKr(const Position& p, Score& res){
	color Pcolor=p.getBitmap(Position::whitePawns)?white:black;
	tSquare pawnSquare;
	if(Pcolor==white){
		pawnSquare = p.getSquareOfThePiece(Position::whitePawns);
		if(	FILES[pawnSquare]== FILES[p.getSquareOfThePiece(Position::blackKing)]
		    && RANKS[pawnSquare]<=6
		    && RANKS[pawnSquare]<RANKS[p.getSquareOfThePiece(Position::blackKing)]
		){
			res=128;
			//p.display();
			return true;
		}
	}
	else{
		pawnSquare = p.getSquareOfThePiece(Position::blackPawns);
		if(	FILES[pawnSquare]== FILES[p.getSquareOfThePiece(Position::whiteKing)]
			&& RANKS[pawnSquare]>=1
			&& RANKS[pawnSquare]>RANKS[p.getSquareOfThePiece(Position::whiteKing)]
		){
			res=128;
			//p.display();
			return true;
		}

	}
	return false;

}

bool evalKBNvsK(const Position& p, Score& res){
	//p.display();
	color color=p.getBitmap(Position::whiteBishops)?white:black;
	tSquare bishopSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	tSquare mateSquare1,mateSquare2;
	int mul=1;
	if(color==white){
		mul=1;
		bishopSquare = p.getSquareOfThePiece(Position::whiteBishops);
		kingSquare = p.getSquareOfThePiece(Position::whiteKing);
		enemySquare = p.getSquareOfThePiece(Position::blackKing);
	}
	else{
		mul=-1;
		bishopSquare = p.getSquareOfThePiece(Position::blackBishops);
		kingSquare = p.getSquareOfThePiece(Position::blackKing);
		enemySquare = p.getSquareOfThePiece(Position::whiteKing);

	}
	int mateColor =SQUARE_COLOR[bishopSquare];
	if(mateColor== 0){

		mateSquare1=A1;
		mateSquare2=H8;
	}else{
		mateSquare1=A8;
		mateSquare2=H1;
	}

	res= SCORE_KNOWN_WIN+50000;
	//sync_cout<<"dis1="<<SQUARE_DISTANCE[enemySquare][kingSquare]<<sync_endl;
	//sync_cout<<"dis2="<<SQUARE_DISTANCE[enemySquare][mateSquare1]<<sync_endl;
	//sync_cout<<"dis3="<<SQUARE_DISTANCE[enemySquare][mateSquare2]<<sync_endl;
	res -= 5*SQUARE_DISTANCE[enemySquare][kingSquare];// devo tenere il re vicino
	res -= 10*std::min(SQUARE_DISTANCE[enemySquare][mateSquare1],SQUARE_DISTANCE[enemySquare][mateSquare2]);// devo portare il re avversario nel giusto angolo



	res *=mul;
	//sync_cout<<"RES:"<<res<<sync_endl;
	return true;

}

bool evalKQvsK(const Position& p, Score& res){
	//p.display();
	color color=p.getBitmap(Position::whiteQueens)?white:black;
	tSquare queenSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	int mul=1;
	if(color==white){
		mul=1;
		queenSquare = p.getSquareOfThePiece(Position::whiteQueens);
		kingSquare = p.getSquareOfThePiece(Position::whiteKing);
		enemySquare = p.getSquareOfThePiece(Position::blackKing);
	}
	else{
		mul=-1;
		queenSquare = p.getSquareOfThePiece(Position::blackQueens);
		kingSquare = p.getSquareOfThePiece(Position::blackKing);
		enemySquare = p.getSquareOfThePiece(Position::whiteKing);

	}


	res= SCORE_KNOWN_WIN+50000;
	res -= 5*SQUARE_DISTANCE[enemySquare][kingSquare];// devo tenere il re vicino
	res -= 5*SQUARE_DISTANCE[enemySquare][queenSquare];// devo portare il re avversario nel giusto angolo



	res *=mul;
	return true;

}

bool kingsDirectOpposition(const Position& p)
{
	if(
			(p.getSquareOfThePiece(Position::whiteKing)+16 == p.getSquareOfThePiece(Position::blackKing) )
			/*||
			(p.getSquareOfThePiece(Position::whiteKing) == p.getSquareOfThePiece(Position::blackKing) +16 )*/
	)
		return true;

	return false;

}

bool evalKPvsK(const Position& p, Score& res){
	//p.display();
	color color=p.getBitmap(Position::whitePawns)?white:black;
	tSquare pawnSquare;
	tSquare kingSquare;
	tSquare enemySquare;

	if(color==white){
		pawnSquare = p.getSquareOfThePiece(Position::whitePawns);
		kingSquare = p.getSquareOfThePiece(Position::whiteKing);
		enemySquare = p.getSquareOfThePiece(Position::blackKing);


		tSquare promotionSquare=BOARDINDEX[FILES[pawnSquare]][7];
		const int relativeRank =RANKS[pawnSquare];
		// Rule of the square
		if ( std::min( 5, (int)(7- relativeRank)) <  std::max(SQUARE_DISTANCE[enemySquare][promotionSquare] - (p.getNextTurn() == Position::whiteTurn?0:1),0) )
		{
			res = SCORE_KNOWN_WIN + relativeRank;
			return true;
		}
		if(FILES[pawnSquare]!=0 && FILES[pawnSquare]!= 7)
		{

			if(SQUARE_DISTANCE[enemySquare][pawnSquare]>=2 || p.getNextTurn()==Position::whiteTurn)
			{
				//winning king on a key square
				if(relativeRank < 4)
				{
					if(kingSquare >= pawnSquare+15 && kingSquare <= pawnSquare+17 )
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}
				}
				else if(relativeRank < 6)
				{
					if((kingSquare >= pawnSquare+15 && kingSquare <= pawnSquare+17) || (kingSquare >= pawnSquare+7 && kingSquare <= pawnSquare+9))
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}
				}
				else{

					if((kingSquare >= pawnSquare-1 && kingSquare <= pawnSquare+1) || (kingSquare >= pawnSquare+7 && kingSquare <= pawnSquare+9))
					{
						res = SCORE_KNOWN_WIN + relativeRank;
						return true;
					}

				}

				// 3 rules for winning, if  conditions are met -> it's won
				unsigned int count =0;
				if(kingSquare == pawnSquare + 8) count++;
				if(p.getNextTurn() == Position::blackTurn && kingsDirectOpposition(p)) count++;
				if(RANKS[kingSquare]==5) count++;

				if(count>1)
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
			if(abs(FILES[enemySquare] -FILES[pawnSquare])<=1  && RANKS[enemySquare]>5)
			{
				res = 0;
				return true;
			}


		}
	}
	else{
		pawnSquare = p.getSquareOfThePiece(Position::blackPawns);
		kingSquare = p.getSquareOfThePiece(Position::blackKing);
		enemySquare = p.getSquareOfThePiece(Position::whiteKing);



		tSquare promotionSquare=BOARDINDEX[FILES[pawnSquare]][0];
		const int relativeRank =7-RANKS[pawnSquare];
		// Rule of the square
		if ( std::min( 5, (int)(7- relativeRank)) <  std::max(SQUARE_DISTANCE[enemySquare][promotionSquare] - (p.getNextTurn() == Position::blackTurn?0:1),0) )
		{
			res = -SCORE_KNOWN_WIN -relativeRank;
			return true;
		}
		if(FILES[pawnSquare]!=0 && FILES[pawnSquare]!= 7)
		{
			if(SQUARE_DISTANCE[enemySquare][pawnSquare]>=2 || p.getNextTurn() == Position::blackTurn)
			{
				//winning king on a key square
				if(relativeRank < 4)
				{
					if(kingSquare >= pawnSquare-17 && kingSquare <= pawnSquare-15 )
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				else if(relativeRank < 6)
				{
					if((kingSquare >= pawnSquare-17 && kingSquare <= pawnSquare-15) || (kingSquare >= pawnSquare-9 && kingSquare <= pawnSquare-7))
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				else{

					if((kingSquare >= pawnSquare-1 && kingSquare <= pawnSquare+1) || (kingSquare >= pawnSquare-9 && kingSquare <= pawnSquare-7))
					{
						res = -SCORE_KNOWN_WIN - relativeRank;
						return true;
					}
				}
				// 3 rules for winning, if  conditions are met -> it's won
				unsigned int count =0;
				if(kingSquare == pawnSquare - 8) count++;
				if(p.getNextTurn() == Position::whiteTurn && kingsDirectOpposition(p)) count++;
				if(RANKS[kingSquare]==2) count++;

				if(count>1)
				{
					res = -SCORE_KNOWN_WIN - relativeRank;
					return true;
				}
			}
			//draw rule
			if((enemySquare==pawnSquare-8) || (enemySquare==pawnSquare-16 && RANKS[enemySquare]!=0))
			{
				res = 0;
				return true;
			}
		}
		else
		{
			//ROOKS PAWN
			if(abs(FILES[enemySquare] -FILES[pawnSquare])<=1  && RANKS[enemySquare]<2)
			{
				res = 0;
				return true;
			}


		}
	}


	return false;
}


bool evalOppositeBishopEndgame(const Position& p, Score& res){
	if(SQUARE_COLOR[p.getSquareOfThePiece(Position::blackBishops)] !=SQUARE_COLOR[ p.getSquareOfThePiece(Position::whiteBishops)])
	{
		unsigned int pawnCount = 0;
		int pawnDifference = 0;
		unsigned int freePawn = 0;

		bitMap pawns= p.getBitmap(Position::whitePawns);
		while(pawns)
		{
			pawnCount ++;
			pawnDifference++;
			tSquare pawn =iterateBit(pawns);
			if( !(PASSED_PAWN[0][pawn] & p.getBitmap(Position::blackPawns)))
			{
				if(!(Movegen::getBishopPseudoAttack(p.getSquareOfThePiece(Position::blackBishops)) & SQUARES_IN_FRONT_OF[0][pawn] ))
				{
					res = 256;
					freePawn++;
				}
			}
		}

		pawns= p.getBitmap(Position::blackPawns);
		while(pawns)
		{
			pawnCount ++;
			pawnDifference--;

			tSquare pawn =iterateBit(pawns);
			if(!( PASSED_PAWN[1][pawn] & p.getBitmap(Position::whitePawns)))
			{
				if(!(Movegen::getBishopPseudoAttack(p.getSquareOfThePiece(Position::whiteBishops)) & SQUARES_IN_FRONT_OF[1][pawn] ))
				{
					freePawn++;
					res = 256;
				}
			}
		}

		if(freePawn==0 && std::abs(pawnDifference)<=1  )
		{
			res = std::min(20+pawnCount * 25,(unsigned int)256);
			return true;
		}

	}
	return false;

}

bool evalKRvsKm(const Position& , Score& res){
	res=64;
	return true;
}

bool evalKNNvsK(const Position& , Score& res){
	res=10;
	return true;
}



void initMobilityBonus(void){
	for(int i=0;i<Position::separationBitmap;i++){
		for(int n=0;n<32;n++){
			mobilityBonus[i][n] =0;
		}
	}
	for(int n=0;n<32;n++){
		mobilityBonus[Position::Queens][n] =simdScore(queenMobilityPars[2]*(n-queenMobilityPars[0]),queenMobilityPars[3]*(n-queenMobilityPars[1]),0,0);
	}
	for(int n=0;n<32;n++){
		mobilityBonus[Position::Rooks][n] =simdScore(rookMobilityPars[2]*(n-rookMobilityPars[0]),rookMobilityPars[3]*(n-rookMobilityPars[1]),0,0);
	}
	for(int n=0;n<32;n++){
		mobilityBonus[Position::Bishops][n] =simdScore(bishopMobilityPars[2]*(n-bishopMobilityPars[0]),bishopMobilityPars[3]*(n-bishopMobilityPars[1]),0,0);
	}
	for(int n=0;n<32;n++){
		mobilityBonus[Position::Knights][n] =simdScore(knightMobilityPars[2]*(n-knightMobilityPars[0]),knightMobilityPars[3]*(n-knightMobilityPars[1]),0,0);
	}

	mobilityBonus[Position::Knights][0].insert(0,-4000.0);
	mobilityBonus[Position::Knights][1].insert(0,-1000.0);
	mobilityBonus[Position::Knights][2].insert(0,-500.0);
	mobilityBonus[Position::Knights][3].insert(0,0.0);
	mobilityBonus[Position::Knights][4].insert(0,280.0);
	mobilityBonus[Position::Knights][5].insert(0,600.0);
	mobilityBonus[Position::Knights][6].insert(0,900.0);
	mobilityBonus[Position::Knights][7].insert(0,1300.0);
	mobilityBonus[Position::Knights][8].insert(0,1400.0);



}

void initMaterialKeys(void){
	/*---------------------------------------------
	 *
	 * K vs K		->	draw
	 * km vs k		->	draw
	 * km vs km		->	draw
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
	U64 key;
	//------------------------------------------
	//	k vs K
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	materialStruct t;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs K
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs K
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	k vs KB
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	k vs KN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kn vs KB
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KB
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbb vs KN
	//------------------------------------------
	p.setupFromFen("kbb5/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbb vs KB
	//------------------------------------------
	p.setupFromFen("kbb5/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs KN
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs KB
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	knn vs KN
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	knn vs KB
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KBB
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5BBK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KBB
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5BBK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KBN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5BNK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KBN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5BNK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KNN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5NNK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KNN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5NNK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs Kpawns
	//------------------------------------------
	t.type=materialStruct::saturationL;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("kb6/8/8/8/8/8/7P/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/6PP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/5PPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/4PPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/3PPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/2PPPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/1PPPPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/PPPPPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});

	t.type=materialStruct::saturationH;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("k7/8/8/8/8/8/7p/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/6pp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/5ppp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/4pppp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/3ppppp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/2pppppp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/1ppppppp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/ppppppp/6BK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kn vs Kpawns
	//------------------------------------------
	t.type=materialStruct::saturationL;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("kn6/8/8/8/8/8/7P/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/6PP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/5PPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/4PPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/3PPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/2PPPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/1PPPPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/PPPPPPPP/7K w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});

	t.type=materialStruct::saturationH;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("k7/8/8/8/8/8/7p/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/6pp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/5ppp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/4pppp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/3ppppp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/2pppppp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/1ppppppp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/ppppppp/6NK w - -");
	key = p.getMaterialKey();
	materialKeyMap.insert({key,t});







	//------------------------------------------
	//	k vs KBP
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5BPK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbp vs K
	//------------------------------------------
	p.setupFromFen("kbp5/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KBN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5BNK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs K
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KQ
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6QK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKQvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kq vs K
	//------------------------------------------
	p.setupFromFen("kq6/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKQvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KP
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6PK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kp vs K
	//------------------------------------------
	p.setupFromFen("kp6/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});


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
				//sync_cout<<s<<sync_endl;
				p.setupFromFen(s);
				key = p.getMaterialKey();
				t.type=materialStruct::multiplicativeFunction;
				t.pointer=&evalOppositeBishopEndgame;
				t.val=0;
				materialKeyMap.insert({key,t});
			}
		}
	}

	//------------------------------------------
	//	kr vs KN
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/6NK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kr vs KB
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/6BK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KR
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6RK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KR
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6RK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	knn vs K
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/7K w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKNNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KNN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5NNK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKNNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kr vs KRP
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/5PRK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRPvsKr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	krp vs KR
	//------------------------------------------
	p.setupFromFen("krp5/8/8/8/8/8/8/6RK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRPvsKr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kq vs KP
	//------------------------------------------
	p.setupFromFen("kq6/8/8/8/8/8/8/6PK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKQvsKP;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kp vs KQ
	//------------------------------------------
	p.setupFromFen("kp6/8/8/8/8/8/8/6QK w - -");
	key = p.getMaterialKey();
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKQvsKP;
	t.val=0;
	materialKeyMap.insert({key,t});
}


//---------------------------------------------
const materialStruct* getMaterialData(const Position& p)
{
	U64 key = p.getMaterialKey();

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




template<color c>
simdScore evalPawn(const Position & p,tSquare sq, bitMap & weakPawns, bitMap & passedPawns){
	simdScore res=0;

	bool passed, isolated, doubled, opposed, chain, backward;
	const bitMap ourPawns =c?p.getBitmap(Position::blackPawns):p.getBitmap(Position::whitePawns);
	const bitMap theirPawns =c?p.getBitmap(Position::whitePawns):p.getBitmap(Position::blackPawns);

	const int relativeRank =c?7-RANKS[sq] :RANKS[sq];
	//const int file =FILES[sq];

	// Our rank plus previous one. Used for chain detection
	bitMap b = RANKMASK[sq] | RANKMASK[sq-pawnPush(c)];
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
		if(opposed){
			res -= isolatedPawnPenaltyOpp;
		}
		else{
			res -= isolatedPawnPenalty;
		}
		weakPawns|=BITSET[sq];

	}

    if (doubled){
    	res-= doubledPawnPenalty;
	}

    if (backward){
    	if(opposed){
			res -= backwardPawnPenalty/2;
		}
		else{
			res -= backwardPawnPenalty;
		}
		weakPawns|=BITSET[sq];
	}

    if (chain){
    	res+= chainedPawnBonus;
	}
	else
	{
		weakPawns|=BITSET[sq];
	}


	//passed pawn
	if(passed &&!doubled){
		passedPawns|=BITSET[sq];
	}

	if ( !passed && !isolated && !doubled && !opposed && bitCnt( PASSED_PAWN[c][sq] & theirPawns ) < bitCnt(PASSED_PAWN[c][sq-pawnPush(c)] & ourPawns ))
	{
		res+=candidateBonus*(relativeRank-1);
	}
	return res;
}

template<Position::bitboardIndex piece>
simdScore evalPieces(const Position & p, const bitMap * const weakSquares,  bitMap * const attackedSquares ,const bitMap * const holes,bitMap const blockedPawns, bitMap * const kingRing,unsigned int * const kingAttackersCount,unsigned int * const kingAttackersWeight,unsigned int * const kingAdjacentZoneAttacksCount, bitMap & weakPawns){
	simdScore res=0;
	bitMap tempPieces=p.getBitmap(piece);
	bitMap enemyKing =(piece>Position::separationBitmap)? p.getBitmap(Position::whiteKing):p.getBitmap(Position::blackKing);
	tSquare enemyKingSquare =(piece>Position::separationBitmap)? p.getSquareOfThePiece(Position::whiteKing):p.getSquareOfThePiece(Position::blackKing);
	bitMap enemyKingRing =(piece>Position::separationBitmap)? kingRing[white]:kingRing[black];
	unsigned int * pKingAttackersCount=(piece>Position::separationBitmap)?&kingAttackersCount[black]:&kingAttackersCount[white];
	unsigned int * pkingAttackersWeight=(piece>Position::separationBitmap)?&kingAttackersWeight[black]:&kingAttackersWeight[white];
	unsigned int * pkingAdjacentZoneAttacksCount=(piece>Position::separationBitmap)?&kingAdjacentZoneAttacksCount[black]:&kingAdjacentZoneAttacksCount[white];
	const bitMap & enemyWeakSquares=(piece>Position::separationBitmap)? weakSquares[white]:weakSquares[black];
	const bitMap & enemyHoles=(piece>Position::separationBitmap)? holes[white]:holes[black];
	const bitMap & supportedSquares=(piece>Position::separationBitmap)? attackedSquares[Position::blackPawns]:attackedSquares[Position::whitePawns];
	const bitMap & threatenSquares=(piece>Position::separationBitmap)? attackedSquares[Position::whitePawns]:attackedSquares[Position::blackPawns];
	const bitMap ourPieces=(piece>Position::separationBitmap)? p.getBitmap(Position::blackPieces):p.getBitmap(Position::whitePieces);
	const bitMap ourPawns=(piece>Position::separationBitmap)? p.getBitmap(Position::blackPawns):p.getBitmap(Position::whitePawns);
	const bitMap theirPieces=(piece>Position::separationBitmap)? p.getBitmap(Position::whitePieces):p.getBitmap(Position::blackPieces);
	const bitMap theirPawns=(piece>Position::separationBitmap)? p.getBitmap(Position::whitePawns):p.getBitmap(Position::blackPawns);

	(void)theirPawns;

	while(tempPieces){
		tSquare sq=iterateBit(tempPieces);
		unsigned int relativeRank =(piece>Position::separationBitmap)? 7-RANKS[sq]:RANKS[sq];

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti



		bitMap attack;


		switch(piece){

			case Position::whiteRooks:
				attack = Movegen::attackFrom<piece>(sq,p.getOccupationBitmap()^p.getBitmap(Position::whiteRooks)^p.getBitmap(Position::whiteQueens));
				break;
			case Position::blackRooks:
				attack = Movegen::attackFrom<piece>(sq,p.getOccupationBitmap()^p.getBitmap(Position::blackRooks)^p.getBitmap(Position::blackQueens));
				break;
			case Position::whiteBishops:
				attack = Movegen::attackFrom<piece>(sq,p.getOccupationBitmap()^p.getBitmap(Position::whiteQueens));
				break;
			case Position::blackBishops:
				attack = Movegen::attackFrom<piece>(sq,p.getOccupationBitmap()^p.getBitmap(Position::blackQueens));
				break;
			case Position::whiteQueens:
			case Position::blackQueens:
				attack = Movegen::attackFrom<piece>(sq,p.getOccupationBitmap());
				break;
			case Position::whiteKnights:
			case Position::blackKnights:

				attack = Movegen::attackFrom<piece>(sq,p.getOccupationBitmap());
				break;
			default:
				break;
		}

		if(attack & enemyKingRing){
			(*pKingAttackersCount)++;
			(*pkingAttackersWeight)+=KingAttackWeights[piece%Position::separationBitmap];
			bitMap adjacent=attack& Movegen::attackFrom<Position::whiteKing>(enemyKingSquare);
			if(adjacent)
			{
				(*pkingAdjacentZoneAttacksCount)+=bitCnt(adjacent);
			}
		}
		attackedSquares[piece]|=attack;


		bitMap defendedPieces=attack&ourPieces&~ourPawns;
		// piece coordination
		res+=bitCnt(defendedPieces)*pieceCoordination;


		//unsigned int mobility= (bitCnt(attack&~(threatenSquares|ourPieces))+ bitCnt(attack&~(ourPieces)))/2;
		unsigned int mobility= bitCnt(attack&~(threatenSquares|ourPieces));
		//sync_cout<<mobility<<sync_endl;
		res+=mobilityBonus[piece%Position::separationBitmap][mobility];
		if(piece!=Position::whiteKnights && piece!=Position::blackKnights)
		if(!(attack&~(threatenSquares|ourPieces)) && (threatenSquares&bitSet(sq))){ // zero mobility && attacked by pawn
			res-=(Position::pieceValue[piece]/4);
		}
		/////////////////////////////////////////
		// center control
		/////////////////////////////////////////
		if(attack & centerBitmap){
			res+=bitCnt(attack & centerBitmap)*piecesCenterControl;
		}
		if(attack & bigCenterBitmap){
			res+=bitCnt(attack & bigCenterBitmap)*piecesBigCenterControl;
		}

		switch(piece){
		case Position::whiteQueens:
		case Position::blackQueens:
		{
			bitMap enemyBackRank=(piece>Position::separationBitmap)? RANKMASK[A1]:RANKMASK[A8];
			bitMap enemyPawns=(piece>Position::separationBitmap)? p.getBitmap(Position::whitePawns):p.getBitmap(Position::blackPawns);
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
			bitMap enemyPawns=(piece>Position::separationBitmap)? p.getBitmap(Position::whitePawns):p.getBitmap(Position::blackPawns);
			bitMap ourPawns=(piece>Position::separationBitmap)? p.getBitmap(Position::blackPawns):p.getBitmap(Position::whitePawns);
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
			//--------------------------------
			// torre intrappolata
			//--------------------------------
			else if(mobility<3)
			{

				tSquare ksq= (piece<=Position::separationBitmap)? p.getSquareOfThePiece(Position::whiteKing):p.getSquareOfThePiece(Position::blackKing);
				unsigned int relativeRankKing =(piece>Position::separationBitmap)? 7-RANKS[ksq]:RANKS[ksq];


				if(
					((FILES[ksq] < 4) == (FILES[sq] < FILES[ksq])) &&
					(RANKS[ksq] == RANKS[sq] && relativeRankKing == 0)
				)
				{

					res -= rookTrapped*(3-mobility);
					Position::state & st = p.getActualState();
					if(piece<=Position::separationBitmap)
					{
						if(!(st.castleRights & (Position::wCastleOO| Position::wCastleOOO)) )
						{
							res-= rookTrappedKingWithoutCastlig*(3-mobility) ;
						}
					}
					else
					{
						if(!(st.castleRights & (Position::bCastleOO| Position::bCastleOOO)) )
						{
							res-= rookTrappedKingWithoutCastlig*(3-mobility);
						}

					}
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
			// alfiere cattivo
			{
				int color =SQUARE_COLOR[sq];
				bitMap blockingPawns=ourPieces & blockedPawns & BITMAP_COLOR[color];
				if(moreThanOneBit(blockingPawns)){
					res-=bitCnt(blockingPawns)*badBishop;
				}
			}

			break;
		case Position::whiteKnights:
		case Position::blackKnights:
			if(enemyWeakSquares& BITSET[sq])
			{
				res+=knightOnOutpost*(5-std::abs((int)relativeRank-5));
				if(supportedSquares &BITSET[sq]){
					res += knightOnOutpostSupported;
				}
				if(enemyHoles &BITSET[sq]){
					res += knightOnHole;
				}

			}
			{
			bitMap wpa=attack & (weakPawns) & theirPieces;
				if(wpa)
				{
					res += bitCnt(wpa)*KnightAttackingWeakPawn;
				}
			}
			break;
		default:
			break;
		}
	}
	return res;
}

template<color c>
Score evalShieldStorm(const Position &pos, tSquare ksq){
	Score ks=0;
	const bitMap ourPawns =c?pos.getBitmap(Position::blackPawns):pos.getBitmap(Position::whitePawns);
	const bitMap theirPawns =c?pos.getBitmap(Position::whitePawns):pos.getBitmap(Position::blackPawns);
	const unsigned int disableRank= c? 0: 7;
	bitMap localKingRing=Movegen::attackFrom<Position::whiteKing>(ksq);
	bitMap localKingShield=localKingRing;
	if(RANKS[ksq]!=disableRank)
	{
	localKingRing|=Movegen::attackFrom<Position::whiteKing>(ksq+pawnPush(c));
	}
	bitMap localKingFarShield=localKingRing&~(localKingShield);

	bitMap pawnShield=localKingShield&ourPawns;
	bitMap pawnFarShield=localKingFarShield&ourPawns;
	bitMap pawnStorm=PASSED_PAWN[c][ksq]&theirPawns;
	if(pawnShield){
		ks=bitCnt(pawnShield)*kingShieldBonus[0];
	}
	if(pawnFarShield){
		ks+=bitCnt(pawnFarShield)*kingFarShieldBonus[0];
	}
	while(pawnStorm){
		tSquare p=iterateBit(pawnStorm);
		ks-=(8-SQUARE_DISTANCE[p][ksq])*kingStormBonus[0];
	}
	return ks;
}
/*
template<color c>
Score evalKingSafety(const Position &pos, Score kingSafety, unsigned int kingAttackersCount, unsigned int kingAdjacentZoneAttacksCount, unsigned int kingAttackersWeight, bitMap * const attackedSquares)
{
	//bitMap * OurPieces = c ? &pos.getBitmap(Position::separationBitmap) : &pos.getBitmap(Position::occupiedSquares);
	bitMap TheirPieces = c ? pos.getBitmap(Position::whitePieces) : pos.getBitmap(Position::blackPieces);
	bitMap * TheirPieces = c ? &attackedSquares[Position::separationBitmap] : &attackedSquares[Position::occupiedSquares];
	bitMap * AttackedByTheirPieces = c ? &attackedSquares[Position::occupiedSquares] : &attackedSquares[Position::separationBitmap];
	Position::bitboardIndex kingSquare = c ? pos.getSquareOfThePiece(Position::blackKing) : pos.getSquareOfThePiece(Position::whiteKing);

	if( kingAttackersCount )// se il re e' attaccato
	{

		bitMap undefendedSquares = AttackedByTheirPieces[Position::Pieces] & AttackedByOurPieces[Position::King];
		undefendedSquares &=
			~(AttackedByOurPieces[Position::Pawns]
			| AttackedByOurPieces[Position::Knights]
			| AttackedByOurPieces[Position::Bishops]
			| AttackedByOurPieces[Position::Rooks]
			| AttackedByOurPieces[Position::Queens]);

		signed int attackUnits =  std::min((unsigned int)25, (kingAttackersCount * kingAttackersWeight) / 2)
							 + 3 * (kingAdjacentZoneAttacksCount + bitCnt( undefendedSquares ) )
							 + KingExposed[ kingSquare ]
							 - kingSafety / kingSafetyScaling[0];

		// safe contact queen check
		bitMap safeContactSquare = undefendedSquares & AttackedByTheirPieces[Position::Queens]  & ~TheirPieces;
		safeContactSquare &= (AttackedByTheirPieces[Position::Rooks]| AttackedByTheirPieces[Position::Bishops] | AttackedByTheirPieces[Position::Knights]| AttackedByTheirPieces[Position::Pawns]);
		if(safeContactSquare)
		{
			attackUnits += 20 * bitCnt(safeContactSquare);
		}
		// safe contact rook check

		safeContactSquare=undefendedSquares & attackedSquares[blackRooks] & ~getBitmap(blackPieces);
		safeContactSquare &= (attackedSquares[blackRooks]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(getSquareOfThePiece(whiteKing));

		if(safeContactSquare){
			attackUnits+=15*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFrom<Position::whiteRooks>(getSquareOfThePiece(whiteKing),getOccupationBitmap());
		bitMap bMap=Movegen::attackFrom<Position::whiteBishops>(getSquareOfThePiece(whiteKing),getOccupationBitmap());

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[blackRooks]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~getBitmap(blackPieces);
		if(longDistCheck){
			attackUnits+=8*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[blackBishops]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~getBitmap(blackPieces);
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFrom<Position::whiteKnights>(getSquareOfThePiece(whiteKing)) & (attackedSquares[blackKnights]) & ~attackedSquares[whitePieces] & ~getBitmap(blackPieces);
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}

		attackUnits = std::max(attackUnits,0);
		attackUnits = std::min(KingSafetyMaxAttack[0], std::max(0, attackUnits));
		attackUnits*=std::min(KingSafetyLinearCoefficent[0], attackUnits);
		attackUnits=std::min(KingSafetyMaxResult[0], attackUnits);
		res-=(kingSafetyBonus*attackUnits);
		if(trace){
			bScore +=(kingSafetyBonus*attackUnits);
		}


	}
}*/

/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
template<bool trace>
Score Position::eval(void) {


	state &st =getActualState();

	//traceRes=0;
	if(trace){

		sync_cout <<std::setprecision(3)<< std::setw(21) << "Eval term " << "|     White    |     Black    |      Total     \n"
			      <<             "                     |    MG    EG  |   MG     EG  |   MG      EG   \n"
			      <<             "---------------------+--------------+--------------+-----------------"<<sync_endl;
	}

	Score lowSat=-SCORE_INFINITE;
	Score highSat=SCORE_INFINITE;
	Score mulCoeff=256;

	bitMap attackedSquares[lastBitboard]={0};
	bitMap weakSquares[2]/*={0}*/;
	bitMap holes[2]/*={0}*/;
	bitMap kingRing[2]/*={0}*/;
	bitMap kingShield[2]/*={0}*/;
	bitMap kingFarShield[2]/*={0}*/;
	unsigned int kingAttackersCount[2]={0};
	unsigned int kingAttackersWeight[2]={0};
	unsigned int kingAdjacentZoneAttacksCount[2]={0};

	tSquare k=getSquareOfThePiece(whiteKing);
	kingRing[white]=Movegen::attackFrom<Position::whiteKing>(k);
	kingShield[white]=kingRing[white];
	if(RANKS[k]<7){kingRing[white]|=Movegen::attackFrom<Position::whiteKing>(tSquare(k+8));}
	kingFarShield[white]=kingRing[white]&~(kingShield[white]|BITSET[k]);


	k=getSquareOfThePiece(blackKing);
	kingRing[black]=Movegen::attackFrom<Position::whiteKing>(k);
	kingShield[black]=kingRing[black];
	if(RANKS[k]>0){kingRing[black]|=Movegen::attackFrom<Position::whiteKing>(tSquare(k-8));}
	kingFarShield[black]=kingRing[black]&~(kingShield[black]|BITSET[k]);




	// todo modificare valori material value & pst
	// material + pst

	simdScore res=simdScore(st.material[0],st.material[1],0,0);


	//-----------------------------------------------------
	//	material evalutation
	//-----------------------------------------------------
	const materialStruct* materialData=getMaterialData(*this);
	if(materialData)
	{
		switch(materialData->type){
		case materialStruct::exact:
			if(st.nextMove)
			{
				return -materialData->val;
			}
			else{
				return materialData->val;
			}
			break;
		case materialStruct::multiplicativeFunction:{
			Score r;
			if(materialData->pointer(*this,r)){
				mulCoeff=r;
			}

			break;
		}
		case materialStruct::exactFunction:
		{	Score r = 0;

			if(materialData->pointer(*this,r)){
				if(st.nextMove)
				{
					return -r;
				}
				else{
					return r;
				}
			}
			break;
		}
		case materialStruct::saturationH:
			highSat=materialData->val;
			break;
		case materialStruct::saturationL:
			lowSat=materialData->val;
					break;
		}
	}

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

	if(trace){
		sync_cout << std::setw(20) << "Material, PST, Tempo" << " |   ---    --- |   ---    --- | "
		          << std::setw(6)  << res[0]/10000.0 << " "
		          << std::setw(6)  << res[1]/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//---------------------------------------------
	//	imbalancies
	//---------------------------------------------
	//	bishop pair

	if(getpieceCount(whiteBishops)>=2 ){
		if(getpieceCount(whiteBishops)==2 && SQUARE_COLOR[getSquareOfThePiece(whiteBishops)]==SQUARE_COLOR[getSquareOfThePiece(whiteBishops,1)]){
		}
		else{
			res+=bishopPair;
		}
	}

	if(getpieceCount(blackBishops)>=2 ){
		if(getpieceCount(blackBishops)==2 && SQUARE_COLOR[getSquareOfThePiece(blackBishops)]==SQUARE_COLOR[getSquareOfThePiece(blackBishops,1)]){
		}
		else{
			res-=bishopPair;
		}
	}
	if((int)getpieceCount(whiteQueens)-(int)getpieceCount(blackQueens)==1
			&& (int)getpieceCount(blackRooks)-(int)getpieceCount(whiteRooks)==1
			&& (int)getpieceCount(blackBishops) +(int)getpieceCount(blackKnights)-(int)getpieceCount(whiteBishops) -(int)getpieceCount(whiteKnights)==2)
	{
		res -= queenVsRook2MinorsImbalance;

	}
	else if((int)getpieceCount(whiteQueens)-(int)getpieceCount(blackQueens)==-1
			&& (int)getpieceCount(blackRooks)-(int)getpieceCount(whiteRooks)==-1
			&& (int)getpieceCount(blackBishops) +(int)getpieceCount(blackKnights)-(int)getpieceCount(whiteBishops) -(int)getpieceCount(whiteKnights)==-2)
	{
		res += queenVsRook2MinorsImbalance;

	}

	if(trace){
		sync_cout << std::setw(20) << "imbalancies" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}





	//todo specialized endgame & scaling function
	//todo material imbalance
	bitMap weakPawns=0;
	bitMap passedPawns=0;




	//----------------------------------------------
	//	PAWNS EVALUTATION
	//----------------------------------------------
	simdScore pawnResult;
	U64 pawnKey = getPawnKey();
	pawnEntry& probePawn= pawnHashTable.probe(pawnKey);
	if(probePawn.key==pawnKey){
		//evalchacheHit++;
		pawnResult=simdScore(probePawn.res[0],probePawn.res[1],0,0);
		weakPawns=probePawn.weakPawns;
		passedPawns=probePawn.passedPawns;
		attackedSquares[whitePawns]=probePawn.pawnAttacks[0];
		attackedSquares[blackPawns]=probePawn.pawnAttacks[1];
		weakSquares[white]=probePawn.weakSquares[0];
		weakSquares[black]=probePawn.weakSquares[1];
		holes[white]=probePawn.holes[0];
		holes[black]=probePawn.holes[1];


	}
	else
	{


		pawnResult=0;
		bitMap pawns= getBitmap(whitePawns);

		while(pawns){
			tSquare sq=iterateBit(pawns);
			pawnResult+=evalPawn<white>(*this,sq, weakPawns, passedPawns);
		}

		pawns= getBitmap(blackPawns);

		while(pawns){
			tSquare sq=iterateBit(pawns);
			pawnResult-=evalPawn<black>(*this,sq, weakPawns, passedPawns);
		}



		bitMap temp=getBitmap(whitePawns);
		bitMap pawnAttack=(temp & ~(FILEMASK[H1]))<<9;
		pawnAttack|=(temp & ~(FILEMASK[A1]))<<7;

		attackedSquares[whitePawns]=pawnAttack;
		pawnAttack|=pawnAttack<<8;
		pawnAttack|=pawnAttack<<16;
		pawnAttack|=pawnAttack<<32;

		weakSquares[white]=~pawnAttack;


		temp=getBitmap(blackPawns);
		pawnAttack=(temp & ~(FILEMASK[H1]))>>7;
		pawnAttack|=(temp & ~(FILEMASK[A1]))>>9;

		attackedSquares[blackPawns]=pawnAttack;

		pawnAttack|=pawnAttack>>8;
		pawnAttack|=pawnAttack>>16;
		pawnAttack|=pawnAttack>>32;

		weakSquares[black]=~pawnAttack;

		temp=getBitmap(whitePawns)<<8;
		temp|=temp<<8;
		temp|=temp<<16;
		temp|=temp<<32;

		holes[white]= weakSquares[white]&temp;



		temp=getBitmap(blackPawns)>>8;
		temp|=temp>>8;
		temp|=temp>>16;
		temp|=temp>>32;

		holes[black]= weakSquares[black]&temp;
		pawnResult-=(bitCnt(holes[white])-bitCnt(holes[black]))*holesPenalty;

		//evalchacheInsert++;
		pawnHashTable.insert(pawnKey,pawnResult, weakPawns, passedPawns,attackedSquares[whitePawns],attackedSquares[blackPawns],weakSquares[white],weakSquares[black],holes[white],holes[black]);



	}

	res+=pawnResult;
	//---------------------------------------------
	// center control
	//---------------------------------------------

	if(attackedSquares[whitePawns] & centerBitmap){
		res+=bitCnt(attackedSquares[whitePawns] & centerBitmap)*pawnCenterControl;
	}
	if(attackedSquares[whitePawns] & bigCenterBitmap){
		res+=bitCnt(attackedSquares[whitePawns] & bigCenterBitmap)*pawnBigCenterControl;
	}

	if(attackedSquares[blackPawns] & centerBitmap){
		res-=bitCnt(attackedSquares[blackPawns] & centerBitmap)*pawnCenterControl;
	}
	if(attackedSquares[blackPawns] & bigCenterBitmap){
		res-=bitCnt(attackedSquares[blackPawns] & bigCenterBitmap)*pawnBigCenterControl;
	}

	if(trace){
		sync_cout << std::setw(20) << "pawns" << " |   ---    --- |   ---    --- | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//-----------------------------------------
	//	blocked pawns
	//-----------------------------------------

	bitMap blockedPawns=(getBitmap(whitePawns)<<8) & getBitmap(blackPawns);
	blockedPawns|=blockedPawns>>8;



	//-----------------------------------------
	//	pieces
	//-----------------------------------------

	simdScore wScore;
	simdScore bScore;
	wScore=evalPieces<Position::whiteKnights>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	bScore=evalPieces<Position::blackKnights>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "knights" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	wScore=evalPieces<Position::whiteBishops>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	bScore=evalPieces<Position::blackBishops>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "bishops" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	wScore=evalPieces<Position::whiteRooks>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	bScore=evalPieces<Position::blackRooks>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "rooks" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	wScore=evalPieces<Position::whiteQueens>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	bScore=evalPieces<Position::blackQueens>(*this,weakSquares,attackedSquares,holes,blockedPawns,kingRing,kingAttackersCount,kingAttackersWeight,kingAdjacentZoneAttacksCount,weakPawns);
	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "queens" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//sync_cout<<"pieces:"<<res[0]<<":"<<res[1]<<sync_endl;
	attackedSquares[whiteKing]=Movegen::attackFrom<Position::whiteKing>(getSquareOfThePiece(whiteKing));
	attackedSquares[blackKing]=Movegen::attackFrom<Position::blackKing>(getSquareOfThePiece(blackKing));

	attackedSquares[whitePieces]=attackedSquares[whiteKing]
								| attackedSquares[whiteKnights]
								| attackedSquares[whiteBishops]
								| attackedSquares[whiteRooks]
								| attackedSquares[whiteQueens]
								| attackedSquares[whitePawns];

	attackedSquares[blackPieces]=attackedSquares[blackKing]
								| attackedSquares[blackKnights]
								| attackedSquares[blackBishops]
								| attackedSquares[blackRooks]
								| attackedSquares[blackQueens]
								| attackedSquares[blackPawns];


	//-----------------------------------------
	//	passed pawn evalutation
	//-----------------------------------------
	// white passed pawns
	bitMap pp=passedPawns&getBitmap(whitePawns);

	wScore=0;
	while(pp){
		simdScore passedPawnsBonus;
		tSquare ppSq=iterateBit(pp);

		unsigned int relativeRank=RANKS[ppSq];

		int r=relativeRank-1;
		int rr= r*(r-1);


		passedPawnsBonus=simdScore(passedPawnBonus[0]*rr,passedPawnBonus[1]*(rr+r+1),0,0);

		if(rr){
			tSquare blockingSquare=ppSq+pawnPush(white);
			// bonus for king proximity to blocking square
			passedPawnsBonus+=enemyKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][getSquareOfThePiece(blackKing)]*rr);
			passedPawnsBonus-=ownKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][getSquareOfThePiece(whiteKing)]*rr);

			if(getPieceAt(blockingSquare)==empty){
				bitMap forwardSquares=SQUARES_IN_FRONT_OF[0][ppSq];
				bitMap backWardSquares=SQUARES_IN_FRONT_OF[1][ppSq];
				bitMap defendedSquares = forwardSquares & attackedSquares[whitePieces];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[blackPieces] |getBitmap(blackPieces) );

				if(unsafeSquares){
					passedPawnsBonus-=passedPawnUnsafeSquares*rr;
					if(unsafeSquares & bitSet(blockingSquare)){
							passedPawnsBonus-=passedPawnBlockedSquares*rr;
					}
				}
				if(defendedSquares){
					passedPawnsBonus+=passedPawnDefendedSquares*rr*bitCnt(defendedSquares);
					if(defendedSquares & bitSet(blockingSquare)){
						passedPawnsBonus+=passedPawnDefendedBlockingSquare*rr;
					}
				}
				if(backWardSquares & getBitmap(whiteRooks))
				{
					passedPawnsBonus+=rookBehindPassedPawn*rr;
				}
				if(backWardSquares & getBitmap(blackRooks))
				{
					passedPawnsBonus-=EnemyRookBehindPassedPawn*rr;
				}

			}
		}

		if(FILES[ppSq]==0 || FILES[ppSq]==7){
			passedPawnsBonus -=passedPawnFileAHPenalty;
		}
		bitMap supportingPawns=getBitmap(whitePawns)&ISOLATED_PAWN[ppSq];
		if(supportingPawns & RANKMASK[ppSq]){
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if(supportingPawns & RANKMASK[ppSq-pawnPush(0)]){
			passedPawnsBonus+=passedPawnSupportedBonus*(r/2);
		}

		if(st.nonPawnMaterial[2]==0){
			tSquare promotionSquare=BOARDINDEX[FILES[ppSq]][7];
			if ( std::min( 5, (int)(7- relativeRank)) <  std::max(SQUARE_DISTANCE[getSquareOfThePiece(blackKing)][promotionSquare] - (st.nextMove==whiteTurn?0:1),0) )
			{
				passedPawnsBonus+=unstoppablePassed*rr;
			}
		}


		wScore+=passedPawnsBonus;

	}
	pp=passedPawns&getBitmap(blackPawns);

	bScore=0;
	while(pp){
		simdScore passedPawnsBonus;
		tSquare ppSq=iterateBit(pp);

		unsigned int relativeRank=7-RANKS[ppSq];

		int r=relativeRank-1;
		int rr= r*(r-1);

		passedPawnsBonus=simdScore(passedPawnBonus[0]*rr,passedPawnBonus[1]*(rr+r+1),0,0);

		if(rr){
			tSquare blockingSquare=ppSq+pawnPush(black);

			// bonus for king proximity to blocking square
			passedPawnsBonus+=enemyKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][getSquareOfThePiece(whiteKing)]*rr);
			passedPawnsBonus-=ownKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][getSquareOfThePiece(blackKing)]*rr);

			if(getPieceAt(blockingSquare)==empty){
				bitMap forwardSquares=SQUARES_IN_FRONT_OF[1][ppSq];
				bitMap backWardSquares=SQUARES_IN_FRONT_OF[0][ppSq];
				bitMap defendedSquares = forwardSquares & attackedSquares[blackPieces];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[whitePieces] | getBitmap(whitePieces));
				if(unsafeSquares){
					passedPawnsBonus-=passedPawnUnsafeSquares*rr;
					if(unsafeSquares & bitSet(blockingSquare)){
							passedPawnsBonus-=passedPawnBlockedSquares*rr;
					}
				}

				if(defendedSquares){
					passedPawnsBonus+=passedPawnDefendedSquares*rr*bitCnt(defendedSquares);
					if(defendedSquares & bitSet(blockingSquare)){
						passedPawnsBonus+=passedPawnDefendedBlockingSquare*rr;
					}
				}
				if(backWardSquares & getBitmap(blackRooks))
				{
					passedPawnsBonus+=rookBehindPassedPawn*rr;
				}
				if(backWardSquares & getBitmap(whiteRooks))
				{
					passedPawnsBonus-=EnemyRookBehindPassedPawn*rr;
				}
			}
		}

		if(FILES[ppSq]==0 || FILES[ppSq]==7){
			passedPawnsBonus -=passedPawnFileAHPenalty;
		}
		bitMap supportingPawns=getBitmap(blackPawns)&ISOLATED_PAWN[ppSq];
		if(supportingPawns & RANKMASK[ppSq]){
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if(supportingPawns & RANKMASK[ppSq-pawnPush(1)]){
			passedPawnsBonus+=passedPawnSupportedBonus*(r/2);
		}

		if(st.nonPawnMaterial[0]==0){
			tSquare promotionSquare=BOARDINDEX[FILES[ppSq]][0];
			if ( std::min( 5, (int)(7- relativeRank)) < std::max(SQUARE_DISTANCE[getSquareOfThePiece(whiteKing)][promotionSquare] - (st.nextMove==whiteTurn?1:0),0) )
			{
				passedPawnsBonus+=unstoppablePassed*rr;

			}

		}

		bScore+=passedPawnsBonus;

	}
	res+=wScore-bScore;

	if(trace){
		sync_cout << std::setw(20) << "passed pawns" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	//sync_cout<<"passedPawns:"<<res[0]<<":"<<res[1]<<sync_endl;

	//todo attacked squares

	//---------------------------------------
	//	space
	//---------------------------------------
	// white pawns
	bitMap spacew =getBitmap(whitePawns);
	spacew|=spacew>>8;
	spacew|=spacew>>16;
	spacew|=spacew>>32;
	spacew &=~attackedSquares[blackPieces];
	//displayBitmap(spacew);



	// black pawns
	bitMap spaceb =getBitmap(blackPawns);
	spaceb|=spaceb<<8;
	spaceb|=spaceb<<16;
	spaceb|=spaceb<<32;
	spaceb &=~attackedSquares[whitePieces];

	//displayBitmap(spaceb);
	res+=(bitCnt(spacew)-bitCnt(spaceb))*spaceBonus;

	if(trace){
		wScore=bitCnt(spacew)*spaceBonus;
		bScore=bitCnt(spaceb)*spaceBonus;
		sync_cout << std::setw(20) << "space" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}



	//sync_cout<<"space:"<<res[0]<<":"<<res[1]<<sync_endl;

	//todo counterattack??

	//todo weakpawn

	//--------------------------------------
	//	weak pieces
	//--------------------------------------
	wScore=0;
	bScore=0;
	bitMap pawnAttackedPieces=getBitmap(whitePieces) & attackedSquares[blackPawns];
	while(pawnAttackedPieces){
		tSquare attacked=iterateBit(pawnAttackedPieces);
		wScore-=attackedByPawnPenalty[getPieceAt(attacked)%separationBitmap];
	}


	// todo fare un weak piece migliore:qualsiasi pezzo attaccato riceve un malus dipendente dal suo più debole attaccante e dal suo valore.
	// volendo anche da quale pezzo è difeso
	bitMap weakPieces=getBitmap(whitePieces) & attackedSquares[blackPieces] &~attackedSquares[whitePawns];
	bitMap undefendedMinors =  (getBitmap(whiteKnights) | getBitmap(whiteBishops))  & ~attackedSquares[whitePieces];
	if (undefendedMinors){
		wScore-=undefendedMinorPenalty;
	}
	while(weakPieces){
		tSquare p=iterateBit(weakPieces);

		bitboardIndex attackedPiece= getPieceAt(p);
		bitboardIndex attackingPiece=blackPawns;
		for(;attackingPiece>=separationBitmap;attackingPiece=(bitboardIndex)(attackingPiece-1)){
			if(attackedSquares[attackingPiece] & bitSet(p)){
				wScore-=weakPiecePenalty[attackedPiece%separationBitmap][attackingPiece%separationBitmap];
				break;
			}
		}
	}





	pawnAttackedPieces=getBitmap(blackPieces) & attackedSquares[whitePawns];
	while(pawnAttackedPieces){
		tSquare attacked=iterateBit(pawnAttackedPieces);
		bScore-=attackedByPawnPenalty[getPieceAt(attacked)%separationBitmap];
	}
	weakPieces=getBitmap(blackPieces) & attackedSquares[whitePieces] &~attackedSquares[blackPawns];
	undefendedMinors =  (getBitmap(blackKnights) | getBitmap(blackBishops))  & ~attackedSquares[blackPieces];
	if (undefendedMinors){
		bScore-=undefendedMinorPenalty;
	}
	while(weakPieces){
		tSquare p=iterateBit(weakPieces);
		bitboardIndex attackedPiece= getPieceAt(p);
		bitboardIndex attackingPiece=whitePawns;
		for(;attackingPiece>=occupiedSquares;attackingPiece=(bitboardIndex)(attackingPiece-1)){
			if(attackedSquares[attackingPiece] & bitSet(p)){
				bScore-=weakPiecePenalty[attackedPiece%separationBitmap][attackingPiece%separationBitmap];
				break;
			}
		}
	}

	// trapped pieces
	// rook trapped on first rank by own king



	res+=wScore-bScore;
	if(trace){
		sync_cout << std::setw(20) << "threat" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}


	//sync_cout<<"weakPieces:"<<res[0]<<":"<<res[1]<<sync_endl;




	//--------------------------------------
	//	king safety
	//--------------------------------------
	wScore=0;
	bScore=0;
	Score kingSafety[2]={0,0};

	kingSafety[0]=evalShieldStorm<white>(*this, getSquareOfThePiece(whiteKing));
	if((st.castleRights & wCastleOO)
		&& !(attackedSquares[blackPieces] & (bitSet(E1)|bitSet(F1)|bitSet(G1) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(F1)|bitSet(G1)))<=1
		){
		kingSafety[0]=std::max(evalShieldStorm<white>(*this, G1),kingSafety[0]);
	}

	if((st.castleRights & wCastleOOO)
		&& !(attackedSquares[blackPieces] & (bitSet(E1)|bitSet(D1)|bitSet(C1) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(D1)|bitSet(C1)|bitSet(B1)))<=1
		){
		kingSafety[0]=std::max(evalShieldStorm<white>(*this, C1),kingSafety[0]);
	}
	wScore=simdScore(kingSafety[0],0,0,0);

	kingSafety[1]=evalShieldStorm<black>(*this, getSquareOfThePiece(blackKing));
	if((st.castleRights & bCastleOO)
		&& !(attackedSquares[whitePieces] & (bitSet(E8)|bitSet(F8)|bitSet(G8) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(F8)|bitSet(G8)))<=1
		){

		kingSafety[1]=std::max(evalShieldStorm<black>(*this, G8),kingSafety[1]);
	}

	if((st.castleRights & bCastleOOO)
		&& !(attackedSquares[whitePieces] & (bitSet(E8)|bitSet(D8)|bitSet(C8) ))
		&& bitCnt(getOccupationBitmap() & (bitSet(D8)|bitSet(C8)|bitSet(B8)))<=1
		){
		kingSafety[1]=std::max(evalShieldStorm<black>(*this, C8),kingSafety[1]);
	}
	bScore=simdScore(kingSafety[1],0,0,0);

	res+=simdScore(kingSafety[0]-kingSafety[1],0,0,0);

	// attacco contro il re nero
	if(kingAttackersCount[white])
	{
		//res+=simdScore(kingSafety[0],0,0,0);

		bitMap undefendedSquares=
			attackedSquares[whitePieces]& attackedSquares[blackKing];
		undefendedSquares&=
			~(attackedSquares[blackPawns]
			|attackedSquares[blackKnights]
			|attackedSquares[blackBishops]
			|attackedSquares[blackRooks]
			|attackedSquares[blackQueens]);


		signed int attackUnits =  std::min((unsigned int)25, (kingAttackersCount[white] * kingAttackersWeight[white]) / 2)
		                     + 3 * (kingAdjacentZoneAttacksCount[white] + bitCnt(undefendedSquares))
		                     + KingExposed[63-getSquareOfThePiece(blackKing)]
		                     - kingSafety[1] / kingSafetyScaling[0];



		// safe contact queen check
		bitMap safeContactSquare=undefendedSquares & attackedSquares[whiteQueens] & ~getBitmap(whitePieces);
		safeContactSquare &= (attackedSquares[whiteRooks]| attackedSquares[whiteBishops] | attackedSquares[whiteKnights]| attackedSquares[whitePawns]);
		if(safeContactSquare){
			attackUnits+=20*bitCnt(safeContactSquare);
		}

		// safe contact rook check
		safeContactSquare=undefendedSquares & attackedSquares[whiteRooks] & ~getBitmap(whitePieces);
		safeContactSquare &= (attackedSquares[whiteQueens]| attackedSquares[whiteBishops] | attackedSquares[whiteKnights]| attackedSquares[whitePawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(getSquareOfThePiece(blackKing));

		if(safeContactSquare){
			attackUnits+=15*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFrom<Position::whiteRooks>(getSquareOfThePiece(blackKing),getOccupationBitmap());
		bitMap bMap=Movegen::attackFrom<Position::whiteBishops>(getSquareOfThePiece(blackKing),getOccupationBitmap());

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[whiteRooks]| attackedSquares[whiteQueens]) & ~attackedSquares[blackPieces] & ~getBitmap(whitePieces);
		if(longDistCheck){
			attackUnits+=8*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[whiteBishops]| attackedSquares[whiteQueens]) & ~attackedSquares[blackPieces] & ~getBitmap(whitePieces);
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFrom<whiteKnights>(getSquareOfThePiece(blackKing)) & (attackedSquares[whiteKnights]) & ~attackedSquares[blackPieces] & ~getBitmap(whitePieces);
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}

		attackUnits = std::max(attackUnits,0);
		attackUnits = std::min(KingSafetyMaxAttack[0], std::max(0, attackUnits));

		attackUnits*=std::min(KingSafetyLinearCoefficent[0], attackUnits);
		attackUnits=std::min(KingSafetyMaxResult[0], attackUnits);

		res+=(kingSafetyBonus*attackUnits);
		if(trace){
			wScore +=(kingSafetyBonus*attackUnits);
		}

	}

	if(kingAttackersCount[black])
	{

		//res-=simdScore(kingSafety[1],0,0,0);
		bitMap undefendedSquares=
			attackedSquares[blackPieces] & attackedSquares[whiteKing];
		undefendedSquares&=
			~(attackedSquares[whitePawns]
			|attackedSquares[whiteKnights]
			|attackedSquares[whiteBishops]
			|attackedSquares[whiteRooks]
			|attackedSquares[whiteQueens]);

		signed int attackUnits =  std::min((unsigned int)25, (kingAttackersCount[black] * kingAttackersWeight[black]) / 2)
							 + 3 * (kingAdjacentZoneAttacksCount[black] + bitCnt(undefendedSquares))
							 + KingExposed[getSquareOfThePiece(whiteKing)]
							 - kingSafety[0] / kingSafetyScaling[0];

		// safe contact queen check
		bitMap safeContactSquare=undefendedSquares & attackedSquares[blackQueens]  & ~getBitmap(blackPieces);
		safeContactSquare &= (attackedSquares[blackRooks]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);
		if(safeContactSquare){
			attackUnits+=20*bitCnt(safeContactSquare);
		}
		// safe contact rook check

		safeContactSquare=undefendedSquares & attackedSquares[blackRooks] & ~getBitmap(blackPieces);
		safeContactSquare &= (attackedSquares[blackQueens]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(getSquareOfThePiece(whiteKing));

		if(safeContactSquare){
			attackUnits+=15*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFrom<Position::whiteRooks>(getSquareOfThePiece(whiteKing),getOccupationBitmap());
		bitMap bMap=Movegen::attackFrom<Position::whiteBishops>(getSquareOfThePiece(whiteKing),getOccupationBitmap());

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[blackRooks]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~getBitmap(blackPieces);
		if(longDistCheck){
			attackUnits+=8*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[blackBishops]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~getBitmap(blackPieces);
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFrom<Position::whiteKnights>(getSquareOfThePiece(whiteKing)) & (attackedSquares[blackKnights]) & ~attackedSquares[whitePieces] & ~getBitmap(blackPieces);
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}

		attackUnits = std::max(attackUnits,0);
		attackUnits = std::min(KingSafetyMaxAttack[0], std::max(0, attackUnits));
		attackUnits*=std::min(KingSafetyLinearCoefficent[0], attackUnits);
		attackUnits=std::min(KingSafetyMaxResult[0], attackUnits);
		res-=(kingSafetyBonus*attackUnits);
		if(trace){
			bScore +=(kingSafetyBonus*attackUnits);
		}


	}


	if(trace)
	{
		sync_cout << std::setw(20) << "king safety" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<< sync_endl;
		sync_cout << "---------------------+--------------+--------------+-----------------" << sync_endl;
		sync_cout << std::setw(20) << "result" << " |"
				  << std::setw(6)  << " " << " "
				  << std::setw(6)  << " "<< " |"
				  << std::setw(6)  << " "<< " "
				  << std::setw(6)  << " " << " | "
				  << std::setw(6)  << (res[0])/10000.0 << " "
				  << std::setw(6)  << (res[1])/10000.0 << " "<< sync_endl;
		traceRes = res;
	}

	//todo scaling
	if(mulCoeff == 256 && (getpieceCount(whitePawns) + getpieceCount(blackPawns) == 0 ) && (abs( st.material[0] )< 40000) && (st.nonPawnMaterial[0]< 90000) && (st.nonPawnMaterial[2] < 90000) )
	{
		mulCoeff=40;
	}
	if(mulCoeff == 256  && st.nonPawnMaterial[0] + st.nonPawnMaterial[2] < 40000  &&  (st.nonPawnMaterial[0] + st.nonPawnMaterial[2] !=0) && (getpieceCount(whitePawns) == getpieceCount(blackPawns)) && !passedPawns )
	{
		mulCoeff = std::min((unsigned int)256, getpieceCount(whitePawns) * 80);
	}






	//--------------------------------------
	//	finalizing
	//--------------------------------------
	signed int gamePhase = getGamePhase();
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



	if(st.nextMove)
	{
		return -score;
	}
	else{
		return score;
	}

}

template Score Position::eval<false>(void);
template Score Position::eval<true>(void);
