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

//unsigned long Position::testPointCounter=0;

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

simdScore ownKingNearPassedPawn=simdScore(10,250,0,0);
simdScore enemyKingNearPassedPawn=simdScore(0,150,0,0);

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
simdScore  kingFarShieldBonus= 1800;
simdScore  kingStormBonus= 80;
simdScore kingSafetyBonus=simdScore(63,-10,0,0);
simdScore  kingSafetyScaling=simdScore(310,0,0,0);
simdScore KingSafetyMaxAttack=simdScore(93,0,0,0);
simdScore KingSafetyLinearCoefficent=simdScore(5,0,0,0);
simdScore KingSafetyMaxResult=simdScore(1000,0,0,0);
//------------------------------------------------


simdScore queenVsRook2MinorsImbalance=simdScore(20000,20000,0,0);


//---------------------------------------------
//	MATERIAL KEYS
//---------------------------------------------


typedef struct{
	enum {
		exact,
		multiplicativeFunction,
		exactFunction,
		saturationH,
		saturationL
	}type ;
	bool (*pointer)(const Position & ,Score &);
	Score val;

}materialStruct;

std::unordered_map<U64, materialStruct> materialKeyMap;

bool evalKBPvsK(const Position& p, Score& res){
	color Pcolor=p.bitBoard[Position::whitePawns]?white:black;
	tSquare pawnSquare;
	tSquare bishopSquare;
	if(Pcolor==white){
		pawnSquare = p.pieceList[Position::whitePawns][0];
		int pawnFile=FILES[pawnSquare];
		if(pawnFile==0 || pawnFile==7){
			bishopSquare=p.pieceList[Position::whiteBishops][0];
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][7]]!=SQUARE_COLOR[bishopSquare]){
				tSquare kingSquare=  p.pieceList[Position::blackKing][0];
				if(RANKS[kingSquare]>=6  && abs(pawnFile-FILES[kingSquare])<=1){
					res=0;
					return true;
				}
			}
		}
	}
	else{
		pawnSquare = p.pieceList[Position::blackPawns][0];
		int pawnFile=FILES[pawnSquare];
		if(pawnFile==0 || pawnFile==7){
			bishopSquare=p.pieceList[Position::blackBishops][0];
			if( SQUARE_COLOR[BOARDINDEX[pawnFile][0]]!=SQUARE_COLOR[bishopSquare]){
				tSquare kingSquare=  p.pieceList[Position::whiteKing][0];
				if(RANKS[kingSquare]<=1  && abs(pawnFile-FILES[kingSquare])<=1){
					res=0;
					return true;

				}
			}
		}
	}
	return false;

}

bool evalKRPvsKr(const Position& p, Score& res){
	color Pcolor=p.bitBoard[Position::whitePawns]?white:black;
	tSquare pawnSquare;
	if(Pcolor==white){
		pawnSquare = p.pieceList[Position::whitePawns][0];
		if(	FILES[pawnSquare]== FILES[p.pieceList[Position::blackKing][0]]
		    && RANKS[pawnSquare]<=6
		    && RANKS[pawnSquare]<RANKS[p.pieceList[Position::blackKing][0]]
		){
			res=128;
			//p.display();
			return true;
		}
	}
	else{
		pawnSquare = p.pieceList[Position::blackPawns][0];
		if(	FILES[pawnSquare]== FILES[p.pieceList[Position::whiteKing][0]]
			&& RANKS[pawnSquare]>=1
			&& RANKS[pawnSquare]>RANKS[p.pieceList[Position::blackKing][0]]
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
	color color=p.bitBoard[Position::whiteBishops]?white:black;
	tSquare bishopSquare;
	tSquare kingSquare;
	tSquare enemySquare;
	tSquare mateSquare1,mateSquare2;
	int mul=1;
	if(color==white){
		mul=1;
		bishopSquare = p.pieceList[Position::whiteBishops][0];
		kingSquare = p.pieceList[Position::whiteKing][0];
		enemySquare = p.pieceList[Position::blackKing][0];
	}
	else{
		mul=-1;
		bishopSquare = p.pieceList[Position::blackBishops][0];
		kingSquare = p.pieceList[Position::blackKing][0];
		enemySquare = p.pieceList[Position::whiteKing][0];

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

bool evalOppositeBishopEndgame(const Position& p, Score& res){
	if(SQUARE_COLOR[p.pieceList[Position::blackBishops][0]] !=SQUARE_COLOR[ p.pieceList[Position::whiteBishops][0]]){
		res=128;
		return true;
	}
	return false;

}

bool evalKRvsKm(const Position& p, Score& res){
	res=64;
	return true;
}

bool evalKNNvsK(const Position& p, Score& res){
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
	 ----------------------------------------------*/


	Position p;
	U64 key;
	//------------------------------------------
	//	k vs K
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	materialStruct t;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs K
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs K
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	k vs KB
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	k vs KN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kn vs KB
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KB
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbb vs KN
	//------------------------------------------
	p.setupFromFen("kbb5/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbb vs KB
	//------------------------------------------
	p.setupFromFen("kbb5/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs KN
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs KB
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	knn vs KN
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/6NK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	knn vs KB
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KBB
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5BBK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KBB
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5BBK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KBN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5BNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KBN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5BNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kb vs KNN
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/5NNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exact;
	t.pointer=nullptr;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KNN
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/5NNK w - -");
	key=p.getActualState().materialKey;
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
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/6PP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/5PPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/4PPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/3PPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/2PPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/1PPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kb6/8/8/8/8/8/PPPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});

	t.type=materialStruct::saturationH;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("k7/8/8/8/8/8/7p/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/6pp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/5ppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/4pppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/3ppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/2pppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/1ppppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/ppppppp/6BK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kn vs Kpawns
	//------------------------------------------
	t.type=materialStruct::saturationL;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("kn6/8/8/8/8/8/7P/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/6PP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/5PPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/4PPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/3PPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/2PPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/1PPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("kn6/8/8/8/8/8/PPPPPPPP/7K w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});

	t.type=materialStruct::saturationH;
	t.pointer=nullptr;
	t.val=0;
	p.setupFromFen("k7/8/8/8/8/8/7p/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/6pp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/5ppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/4pppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/3ppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/2pppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/1ppppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});
	p.setupFromFen("k7/8/8/8/8/8/ppppppp/6NK w - -");
	key=p.getActualState().materialKey;
	materialKeyMap.insert({key,t});







	//------------------------------------------
	//	k vs KBP
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5BPK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbp vs K
	//------------------------------------------
	p.setupFromFen("kbp5/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBPvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KBN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5BNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kbn vs K
	//------------------------------------------
	p.setupFromFen("kbn5/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::exactFunction;
	t.pointer=&evalKBNvsK;
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
				key=p.getActualState().materialKey;
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
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kr vs KB
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/6BK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	kb vs KR
	//------------------------------------------
	p.setupFromFen("kb6/8/8/8/8/8/8/6RK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});
	//------------------------------------------
	//	kn vs KR
	//------------------------------------------
	p.setupFromFen("kn6/8/8/8/8/8/8/6RK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRvsKm;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	knn vs K
	//------------------------------------------
	p.setupFromFen("knn5/8/8/8/8/8/8/7K w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKNNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	k vs KNN
	//------------------------------------------
	p.setupFromFen("k7/8/8/8/8/8/8/5NNK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKNNvsK;
	t.val=0;
	materialKeyMap.insert({key,t});


	//------------------------------------------
	//	kr vs KRP
	//------------------------------------------
	p.setupFromFen("kr6/8/8/8/8/8/8/5PRK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRPvsKr;
	t.val=0;
	materialKeyMap.insert({key,t});

	//------------------------------------------
	//	krp vs KR
	//------------------------------------------
	p.setupFromFen("krp5/8/8/8/8/8/8/6RK w - -");
	key=p.getActualState().materialKey;
	t.type=materialStruct::multiplicativeFunction;
	t.pointer=&evalKRPvsKr;
	t.val=0;
	materialKeyMap.insert({key,t});
}


//---------------------------------------------
const materialStruct* getMaterialData(const Position& p){
	U64 key=p.getActualState().materialKey;

	std::unordered_map<U64,materialStruct>::const_iterator got= materialKeyMap.find(key);
	if(got == materialKeyMap.end())
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
	const bitMap ourPawns =c?p.bitBoard[Position::blackPawns]:p.bitBoard[Position::whitePawns];
	const bitMap theirPawns =c?p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
	bitMap b;
	const int relativeRank =c?7-RANKS[sq] :RANKS[sq];
	//const int file =FILES[sq];

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
	const bitMap ourPieces=(piece>Position::separationBitmap)? p.bitBoard[Position::blackPieces]:p.bitBoard[Position::whitePieces];
	const bitMap ourPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::blackPawns]:p.bitBoard[Position::whitePawns];
	const bitMap theirPieces=(piece>Position::separationBitmap)? p.bitBoard[Position::whitePieces]:p.bitBoard[Position::blackPieces];
	const bitMap theirPawns=(piece>Position::separationBitmap)? p.bitBoard[Position::whitePawns]:p.bitBoard[Position::blackPawns];
	while(tempPieces){
		tSquare sq=firstOne(tempPieces);
		tempPieces&= tempPieces-1;
		unsigned int relativeRank =(piece>Position::separationBitmap)? 7-RANKS[sq]:RANKS[sq];

		//---------------------------
		//	MOBILITY
		//---------------------------
		// todo mobility usare solo mosse valide ( pinned pieces)
		//todo mobility with pinned, mobility contando meno case attaccate da pezzi meno forti



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
				break;
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


		bitMap defendedPieces=attack&ourPieces&~ourPawns;
		// piece coordination
		res+=bitCnt(defendedPieces)*pieceCoordination;


		//unsigned int mobility= (bitCnt(attack&~(threatenSquares|ourPieces))+ bitCnt(attack&~(ourPieces)))/2;
		unsigned int mobility= bitCnt(attack&~(threatenSquares|ourPieces));
		//sync_cout<<mobility<<sync_endl;
		res+=mobilityBonus[piece%Position::separationBitmap][mobility];
		if(piece!=Position::whiteKnights && piece!=Position::blackKnights)
		if(!(attack&~(threatenSquares|ourPieces)) && (threatenSquares&bitSet(sq))){ // zero mobility && attacked by pawn
			res-=(Position::pieceValue[piece%Position::separationBitmap]/4);
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
			//--------------------------------
			// torre intrappolata
			//--------------------------------
			else if(mobility<3)
			{

				tSquare ksq= (piece<=Position::separationBitmap)? p.pieceList[Position::whiteKing][0]:p.pieceList[Position::blackKing][0];
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
				res+=knightOnOutpost*(5-std::abs(relativeRank-5));
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
	const bitMap ourPawns =c?pos.bitBoard[Position::blackPawns]:pos.bitBoard[Position::whitePawns];
	const bitMap theirPawns =c?pos.bitBoard[Position::whitePawns]:pos.bitBoard[Position::blackPawns];
	const unsigned int disableRank= c? 0: 7;
	bitMap localKingRing=Movegen::attackFromKing(ksq);
	bitMap localKingShield=localKingRing;
	if(RANKS[ksq]!=disableRank)
	{
	localKingRing|=Movegen::attackFromKing(ksq+pawnPush(c));
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
		tSquare p=firstOne(pawnStorm);
		ks-=(8-SQUARE_DISTANCE[p][ksq])*kingStormBonus[0];
		pawnStorm&= pawnStorm-1;
	}
	return ks;
}


/*! \brief do a pretty simple evalutation
	\author Marco Belli
	\version 1.0
	\date 27/10/2013
*/
template<bool trace>
Score Position::eval(void) {


	state &st =getActualState();
	evalEntry* probeEval= evalHashTable.probe(st.key);
	if(probeEval!=nullptr){
		if(st.nextMove)
		{
			return -probeEval->res;
		}
		else{
			return probeEval->res;
		}
	}

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
		{	Score r;

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

	if(pieceCount[whiteBishops]>=2 ){
		if(pieceCount[whiteBishops]==2 && SQUARE_COLOR[pieceList[whiteBishops][0]]==SQUARE_COLOR[pieceList[whiteBishops][1]]){
		}
		else{
			res+=bishopPair;
		}
	}

	if(pieceCount[blackBishops]>=2 ){
		if(pieceCount[blackBishops]==2 && SQUARE_COLOR[pieceList[blackBishops][0]]==SQUARE_COLOR[pieceList[blackBishops][1]]){
		}
		else{
			res-=bishopPair;
		}
	}
	if((int)pieceCount[whiteQueens]-(int)pieceCount[blackQueens]==1
			&& (int)pieceCount[blackRooks]-(int)pieceCount[whiteRooks]==1
			&& (int)pieceCount[blackBishops] +(int)pieceCount[blackKnights]-(int)pieceCount[whiteBishops] -(int)pieceCount[whiteKnights]==2)
	{
		res -= queenVsRook2MinorsImbalance;

	}
	else if((int)pieceCount[whiteQueens]-(int)pieceCount[blackQueens]==-1
			&& (int)pieceCount[blackRooks]-(int)pieceCount[whiteRooks]==-1
			&& (int)pieceCount[blackBishops] +(int)pieceCount[blackKnights]-(int)pieceCount[whiteBishops] -(int)pieceCount[whiteKnights]==-2)
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
	pawnEntry* probePawn= pawnHashTable.probe(getActualState().pawnKey);
	if(probePawn!=nullptr){
		pawnResult=simdScore(probePawn->res[0],probePawn->res[1],0,0);
		weakPawns=probePawn->weakPawns;
		passedPawns=probePawn->passedPawns;
		attackedSquares[whitePawns]=probePawn->pawnAttacks[0];
		attackedSquares[blackPawns]=probePawn->pawnAttacks[1];
		weakSquares[white]=probePawn->weakSquares[0];
		weakSquares[black]=probePawn->weakSquares[1];
		holes[white]=probePawn->holes[0];
		holes[black]=probePawn->holes[1];


	}
	else
	{


		pawnResult=0;
		bitMap pawns= bitBoard[whitePawns];

		while(pawns){
			tSquare sq=firstOne(pawns);
			pawnResult+=evalPawn<white>(*this,sq, weakPawns, passedPawns);
			pawns &= pawns-1;
		}

		pawns= bitBoard[blackPawns];

		while(pawns){
			tSquare sq=firstOne(pawns);
			pawnResult-=evalPawn<black>(*this,sq, weakPawns, passedPawns);
			pawns &= pawns-1;
		}



		bitMap temp=bitBoard[whitePawns];
		bitMap pawnAttack=(temp & ~(FILEMASK[H1]))<<9;
		pawnAttack|=(temp & ~(FILEMASK[A1]))<<7;

		attackedSquares[whitePawns]=pawnAttack;
		pawnAttack|=pawnAttack<<8;
		pawnAttack|=pawnAttack<<16;
		pawnAttack|=pawnAttack<<32;

		weakSquares[white]=~pawnAttack;


		temp=bitBoard[blackPawns];
		pawnAttack=(temp & ~(FILEMASK[H1]))>>7;
		pawnAttack|=(temp & ~(FILEMASK[A1]))>>9;

		attackedSquares[blackPawns]=pawnAttack;

		pawnAttack|=pawnAttack>>8;
		pawnAttack|=pawnAttack>>16;
		pawnAttack|=pawnAttack>>32;

		weakSquares[black]=~pawnAttack;

		temp=bitBoard[whitePawns]<<8;
		temp|=temp<<8;
		temp|=temp<<16;
		temp|=temp<<32;

		holes[white]= weakSquares[white]&temp;



		temp=bitBoard[blackPawns]>>8;
		temp|=temp>>8;
		temp|=temp>>16;
		temp|=temp>>32;

		holes[black]= weakSquares[black]&temp;
		pawnResult-=(bitCnt(holes[white])-bitCnt(holes[black]))*holesPenalty;

		pawnHashTable.insert(st.pawnKey,pawnResult, weakPawns, passedPawns,attackedSquares[whitePawns],attackedSquares[blackPawns],weakSquares[white],weakSquares[black],holes[white],holes[black]);



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

	bitMap blockedPawns=(bitBoard[whitePawns]<<8) & bitBoard[blackPawns];
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
	attackedSquares[whiteKing]=Movegen::attackFromKing(pieceList[whiteKing][0]);
	attackedSquares[blackKing]=Movegen::attackFromKing(pieceList[blackKing][0]);

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
	bitMap pp=passedPawns&bitBoard[whitePawns];

	wScore=0;
	while(pp){
		simdScore passedPawnsBonus;
		tSquare ppSq=firstOne(pp);

		pp &= pp-1;

		unsigned int relativeRank=RANKS[ppSq];

		int r=relativeRank-1;
		int rr= r*(r-1);


		passedPawnsBonus=simdScore(passedPawnBonus[0]*rr,passedPawnBonus[1]*(rr+r+1),0,0);

		if(rr){
			tSquare blockingSquare=ppSq+pawnPush(white);
			// bonus for king proximity to blocking square
			passedPawnsBonus+=ownKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][pieceList[blackKing][0]]*rr);
			passedPawnsBonus-=enemyKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][pieceList[whiteKing][0]]*rr);

			if(squares[blockingSquare]==empty){
				bitMap forwardSquares=SQUARES_IN_FRONT_OF[0][ppSq];
				bitMap defendedSquares = forwardSquares & attackedSquares[whitePieces];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[blackPieces] |bitBoard[blackPieces] );

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
			}
		}

		if(FILES[ppSq]==0 || FILES[ppSq]==7){
			passedPawnsBonus -=passedPawnFileAHPenalty;
		}
		bitMap supportingPawns=bitBoard[whitePawns]&ISOLATED_PAWN[ppSq];
		if(supportingPawns & RANKMASK[ppSq]){
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if(supportingPawns & RANKMASK[ppSq-pawnPush(0)]){
			passedPawnsBonus+=passedPawnSupportedBonus*(r/2);
		}

		if(st.nonPawnMaterial[2]==0){
			tSquare promotionSquare=BOARDINDEX[FILES[ppSq]][7];
			if ( std::min( 5, (int)(7- relativeRank)) <  std::max(SQUARE_DISTANCE[pieceList[blackKing][0]][promotionSquare] - (st.nextMove==whiteTurn?0:1),0) )
			{
				passedPawnsBonus+=unstoppablePassed*rr;
			}
		}


		wScore+=passedPawnsBonus;

	}
	pp=passedPawns&bitBoard[blackPawns];

	bScore=0;
	while(pp){
		simdScore passedPawnsBonus;
		tSquare ppSq=firstOne(pp);
		pp &= pp-1;

		unsigned int relativeRank=7-RANKS[ppSq];

		int r=relativeRank-1;
		int rr= r*(r-1);

		passedPawnsBonus=simdScore(passedPawnBonus[0]*rr,passedPawnBonus[1]*(rr+r+1),0,0);

		if(rr){
			tSquare blockingSquare=ppSq+pawnPush(black);

			// bonus for king proximity to blocking square
			passedPawnsBonus+=ownKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][pieceList[whiteKing][0]]*rr);
			passedPawnsBonus-=enemyKingNearPassedPawn*(SQUARE_DISTANCE[blockingSquare][pieceList[blackKing][0]]*rr);

			if(squares[blockingSquare]==empty){
				bitMap forwardSquares=SQUARES_IN_FRONT_OF[1][ppSq];
				bitMap defendedSquares = forwardSquares & attackedSquares[blackPieces];

				bitMap unsafeSquares = forwardSquares & (attackedSquares[whitePieces] | bitBoard[whitePieces]);

				passedPawnsBonus-=passedPawnUnsafeSquares*rr;
				if(unsafeSquares & bitSet(blockingSquare)){
						passedPawnsBonus-=passedPawnBlockedSquares*rr;
				}

				if(defendedSquares){
					passedPawnsBonus+=passedPawnDefendedSquares*rr*bitCnt(defendedSquares);
					if(defendedSquares & bitSet(blockingSquare)){
						passedPawnsBonus+=passedPawnDefendedBlockingSquare*rr;
					}
				}
			}
		}

		if(FILES[ppSq]==0 || FILES[ppSq]==7){
			passedPawnsBonus -=passedPawnFileAHPenalty;
		}
		bitMap supportingPawns=bitBoard[blackPawns]&ISOLATED_PAWN[ppSq];
		if(supportingPawns & RANKMASK[ppSq]){
			passedPawnsBonus+=passedPawnSupportedBonus*r;
		}
		if(supportingPawns & RANKMASK[ppSq-pawnPush(1)]){
			passedPawnsBonus+=passedPawnSupportedBonus*(r/2);
		}

		if(st.nonPawnMaterial[0]==0){
			tSquare promotionSquare=BOARDINDEX[FILES[ppSq]][0];
			if ( std::min( 5, (int)(7- relativeRank)) < std::max(SQUARE_DISTANCE[pieceList[whiteKing][0]][promotionSquare] - (st.nextMove==whiteTurn?1:0),0) )
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
	bitMap spacew =bitBoard[whitePawns];
	spacew|=spacew>>8;
	spacew|=spacew>>16;
	spacew|=spacew>>32;
	spacew &=~attackedSquares[blackPieces];
	//displayBitmap(spacew);



	// black pawns
	bitMap spaceb =bitBoard[blackPawns];
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
	bitMap pawnAttackedPieces=bitBoard[whitePieces] & attackedSquares[blackPawns];
	while(pawnAttackedPieces){
		tSquare attacked=firstOne(pawnAttackedPieces);
		wScore-=attackedByPawnPenalty[squares[attacked]%separationBitmap];
		pawnAttackedPieces &=pawnAttackedPieces-1;
	}


	// todo fare un weak piece migliore:qualsiasi pezzo attaccato riceve un malus dipendente dal suo più debole attaccante e dal suo valore.
	// volendo anche da quale pezzo è difeso
	bitMap weakPieces=bitBoard[whitePieces] & attackedSquares[blackPieces] &~attackedSquares[whitePawns];
	bitMap undefendedMinors =  (bitBoard[whiteKnights] | bitBoard[whiteBishops])  & ~attackedSquares[whitePieces];
	if (undefendedMinors){
		wScore-=undefendedMinorPenalty;
	}
	while(weakPieces){
		tSquare p=firstOne(weakPieces);

		bitboardIndex attackedPiece= squares[p];
		bitboardIndex attackingPiece=blackPawns;
		for(;attackingPiece>=separationBitmap;attackingPiece=(bitboardIndex)(attackingPiece-1)){
			if(attackedSquares[attackingPiece] & bitSet(p)){
				wScore-=weakPiecePenalty[attackedPiece%separationBitmap][attackingPiece%separationBitmap];
				break;
			}
		}
		weakPieces&=weakPieces-1;
	}





	pawnAttackedPieces=bitBoard[blackPieces] & attackedSquares[whitePawns];
	while(pawnAttackedPieces){
		tSquare attacked=firstOne(pawnAttackedPieces);
		bScore-=attackedByPawnPenalty[squares[attacked]%separationBitmap];
		pawnAttackedPieces &=pawnAttackedPieces-1;
	}
	weakPieces=bitBoard[blackPieces] & attackedSquares[whitePieces] &~attackedSquares[blackPawns];
	undefendedMinors =  (bitBoard[blackKnights] | bitBoard[blackBishops])  & ~attackedSquares[blackPieces];
	if (undefendedMinors){
		bScore-=undefendedMinorPenalty;
	}
	while(weakPieces){
		tSquare p=firstOne(weakPieces);
		bitboardIndex attackedPiece= squares[p];
		bitboardIndex attackingPiece=whitePawns;
		for(;attackingPiece>=occupiedSquares;attackingPiece=(bitboardIndex)(attackingPiece-1)){
			if(attackedSquares[attackingPiece] & bitSet(p)){
				bScore-=weakPiecePenalty[attackedPiece%separationBitmap][attackingPiece%separationBitmap];
				break;
			}
		}
		weakPieces&=weakPieces-1;
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

	kingSafety[0]=evalShieldStorm<white>(*this, pieceList[whiteKing][0]);
	if((st.castleRights & wCastleOO)
		&& !(attackedSquares[blackPieces] & (bitSet(E1)|bitSet(F1)|bitSet(G1) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(F1)|bitSet(G1)))
		){
		kingSafety[0]=std::max(evalShieldStorm<white>(*this, G1),kingSafety[0]);
	}

	if((st.castleRights & wCastleOOO)
		&& !(attackedSquares[blackPieces] & (bitSet(E1)|bitSet(D1)|bitSet(C1) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(D1)|bitSet(C1)|bitSet(B1)))
		){
		kingSafety[0]=std::max(evalShieldStorm<white>(*this, C1),kingSafety[0]);
	}
	wScore=simdScore(kingSafety[0],0,0,0);

	kingSafety[1]=evalShieldStorm<black>(*this, pieceList[blackKing][0]);
	if((st.castleRights & bCastleOO)
		&& !(attackedSquares[whitePieces] & (bitSet(E8)|bitSet(F8)|bitSet(G8) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(F8)|bitSet(G8)))
		){

		kingSafety[1]=std::max(evalShieldStorm<black>(*this, G8),kingSafety[1]);
	}

	if((st.castleRights & bCastleOOO)
		&& !(attackedSquares[whitePieces] & (bitSet(E8)|bitSet(D8)|bitSet(C8) ))
		&& !(bitBoard[occupiedSquares] & (bitSet(D8)|bitSet(C8)|bitSet(B8)))
		){
		kingSafety[1]=std::max(evalShieldStorm<black>(*this, C8),kingSafety[1]);
	}
	bScore=simdScore(kingSafety[1],0,0,0);

	res+=simdScore(kingSafety[0]-kingSafety[1],0,0,0);

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
		                     + KingExposed[63-pieceList[blackKing][0]]
		                     - kingSafety[1] / kingSafetyScaling[0];



		// safe contact queen check
		bitMap safeContactSquare=undefendedSquares & attackedSquares[whiteQueens] & ~bitBoard[whitePieces];
		safeContactSquare &= (attackedSquares[whiteRooks]| attackedSquares[whiteBishops] | attackedSquares[whiteKnights]| attackedSquares[whitePawns]);
		if(safeContactSquare){
			attackUnits+=20*bitCnt(safeContactSquare);
		}

		// safe contact rook check
		safeContactSquare=undefendedSquares & attackedSquares[whiteRooks] & ~bitBoard[whitePieces];
		safeContactSquare &= (attackedSquares[whiteRooks]| attackedSquares[whiteBishops] | attackedSquares[whiteKnights]| attackedSquares[whitePawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(pieceList[blackKing][0]);

		if(safeContactSquare){
			attackUnits+=15*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFromRook(pieceList[blackKing][0],bitBoard[occupiedSquares]);
		bitMap bMap=Movegen::attackFromBishop(pieceList[blackKing][0],bitBoard[occupiedSquares]);

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[whiteRooks]| attackedSquares[whiteQueens]) & ~attackedSquares[blackPieces] & ~bitBoard[whitePieces];
		if(longDistCheck){
			attackUnits+=8*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[whiteBishops]| attackedSquares[whiteQueens]) & ~attackedSquares[blackPieces] & ~bitBoard[whitePieces];
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFromKnight(pieceList[blackKing][0]) & (attackedSquares[whiteKnights]) & ~attackedSquares[blackPieces] & ~bitBoard[whitePieces];
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}
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
							 + KingExposed[pieceList[whiteKing][0]]
							 - kingSafety[0] / kingSafetyScaling[0];

		// safe contact queen check
		bitMap safeContactSquare=undefendedSquares & attackedSquares[blackQueens]  & ~bitBoard[blackPieces];
		safeContactSquare &= (attackedSquares[blackRooks]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);
		if(safeContactSquare){
			attackUnits+=20*bitCnt(safeContactSquare);
		}
		// safe contact rook check

		safeContactSquare=undefendedSquares & attackedSquares[blackRooks] & ~bitBoard[blackPieces];
		safeContactSquare &= (attackedSquares[blackRooks]| attackedSquares[blackBishops] | attackedSquares[blackKnights]| attackedSquares[blackPawns]);

		safeContactSquare &=Movegen::getRookPseudoAttack(pieceList[whiteKing][0]);

		if(safeContactSquare){
			attackUnits+=15*bitCnt(safeContactSquare);
		}

		// long distance check
		bitMap rMap=Movegen::attackFromRook(pieceList[whiteKing][0],bitBoard[occupiedSquares]);
		bitMap bMap=Movegen::attackFromBishop(pieceList[whiteKing][0],bitBoard[occupiedSquares]);

		// vertical check
		bitMap longDistCheck=rMap & (attackedSquares[blackRooks]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~bitBoard[blackPieces];
		if(longDistCheck){
			attackUnits+=8*bitCnt(longDistCheck);
		}

		// diagonal check
		longDistCheck=bMap & (attackedSquares[blackBishops]| attackedSquares[blackQueens]) & ~attackedSquares[whitePieces] & ~bitBoard[blackPieces];
		if(longDistCheck){
			attackUnits+=3*bitCnt(longDistCheck);
		}

		///knight check;
		longDistCheck=Movegen::attackFromKnight(pieceList[whiteKing][0]) & (attackedSquares[blackKnights]) & ~attackedSquares[whitePieces] & ~bitBoard[blackPieces];
		if(longDistCheck){
			attackUnits+=bitCnt(longDistCheck);
		}
		attackUnits = std::min(KingSafetyMaxAttack[0], std::max(0, attackUnits));
		attackUnits*=std::min(KingSafetyLinearCoefficent[0], attackUnits);
		attackUnits=std::min(KingSafetyMaxResult[0], attackUnits);
		res-=(kingSafetyBonus*attackUnits);
		if(trace){
			bScore +=(kingSafetyBonus*attackUnits);
		}


	}


	if(trace){
		sync_cout << std::setw(20) << "king safety" << " |"
				  << std::setw(6)  << (wScore[0])/10000.0 << " "
				  << std::setw(6)  << (wScore[1])/10000.0 << " |"
				  << std::setw(6)  << (bScore[0])/10000.0 << " "
				  << std::setw(6)  << (bScore[1])/10000.0 << " | "
				  << std::setw(6)  << (res[0]-traceRes[0])/10000.0 << " "
				  << std::setw(6)  << (res[1]-traceRes[1])/10000.0 << " "<<sync_endl;
		sync_cout <<"---------------------+--------------+--------------+-----------------"<<sync_endl;
		sync_cout << std::setw(20) << "result" << " |"
				  << std::setw(6)  << " " << " "
				  << std::setw(6)  << " "<< " |"
				  << std::setw(6)  << " "<< " "
				  << std::setw(6)  << " " << " | "
				  << std::setw(6)  << (res[0])/10000.0 << " "
				  << std::setw(6)  << (res[1])/10000.0 << " "<<sync_endl;
		traceRes=res;
	}

	//todo scaling
	if(mulCoeff==256 && (pieceCount[whitePawns]+pieceCount[blackPawns]==0) && abs(st.material[0])<40000 && st.nonPawnMaterial[0]<90000 && st.nonPawnMaterial[2]<90000 ){
		mulCoeff=40;
	}





	//--------------------------------------
	//	finalizing
	//--------------------------------------


	signed int gamePhase=getGamePhase();
	signed long long r=((signed long long)res[0])*(65536-gamePhase)+((signed long long)res[1])*gamePhase;

	Score score =(r)/65536;
	if(mulCoeff!=256){
		score*=mulCoeff;
		score/=256;
	}

	// final value saturation
	score=std::min(highSat,score);
	score=std::max(lowSat,score);

	evalHashTable.insert(st.pawnKey,score);


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
