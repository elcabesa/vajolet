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

#include <iomanip>
#include "position.h"
#include "move.h"
#include "bitops.h"
#include "movegen.h"
#include "eval.h"
#include "parameters.h"


simdScore initialPieceValue[Position::lastBitboard] = {
		{0,0,0,0},
		{3000000,3000000,0,0},//king
		{118976,105579,4236,-3888},
		{52614,55708,-6957,7173},
		{33773,37122,-2559,2523},
		{33320,36292,-4615,-4437},
		{7759,9770,-5422,-814},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0}
};

simdScore PawnD3 =  {213,33,-40,54};
simdScore PawnD4 =  {675,-391,156,5};
simdScore PawnD5 =  {574,-217,50,-344};
simdScore PawnE3 =  {100,145,-271,-179};
simdScore PawnE4 =  {715,-362,149,282};
simdScore PawnE5 =  {29,-386,127,-128};
simdScore PawnCentering =  {274,-28,-445,1};
simdScore PawnRankBonus =  {165,357,1,-396};
simdScore KnightPST =  {938,806,-204,70};
simdScore BishopPST =  {142,329,-669,-39};
simdScore RookPST =  {209,-30,-129,142};
simdScore QueenPST =  {184,278,738,95};
simdScore KingPST =  {1120,1084,47,107};

simdScore BishopBackRankOpening =  {923,338,-295,339};
simdScore KnightBackRankOpening =  {2,-200,157,594};
simdScore RookBackRankOpening =  {356,320,178,86};
simdScore QueenBackRankOpening =  {-440,3290,-178,-130};
simdScore BishopOnBigDiagonals =  {563,106,-123,493};



simdScore queenMobilityPars =  {526,439,123,127};
simdScore rookMobilityPars =  {942,1155,509,318};
simdScore bishopMobilityPars =  {-111,391,590,271};
simdScore knightMobilityPars =  {200,841,535,205};
simdScore isolatedPawnPenalty =  {695,1284,-204,3};
simdScore isolatedPawnPenaltyOpp =  {809,1107,-100,164};
simdScore doubledPawnPenalty =  {536,766,-206,192};
simdScore backwardPawnPenalty =  {1024,765,-50,185};
simdScore chainedPawnBonus =  {55,59,66,-191};
simdScore chainedPawnBonusOffset =  {296,1,0,0};
simdScore chainedPawnBonusOpp =  {23,61,66,-191};
simdScore chainedPawnBonusOffsetOpp =  {358,213,0,0};
simdScore passedPawnFileAHPenalty =  {-274,-959,0,0};
simdScore passedPawnSupportedBonus =  {384,210,0,0};
simdScore candidateBonus =  {5,470,0,0};
simdScore passedPawnBonus =  {71,101,0,0};
simdScore passedPawnUnsafeSquares =  {35,-100,0,0};
simdScore passedPawnBlockedSquares =  {-13,219,0,0};
simdScore passedPawnDefendedSquares =  {30,292,0,0};
simdScore passedPawnDefendedBlockingSquare =  {181,301,0,0};
simdScore unstoppablePassed =  {92,611,0,0};
simdScore rookBehindPassedPawn =  {105,487,0,0};
simdScore EnemyRookBehindPassedPawn =  {-30,100,0,0};
simdScore holesPenalty =  {19,-166,-216,83};
simdScore pawnCenterControl =  {472,-125,-8,-69};
simdScore pawnBigCenterControl =  {61,194,-129,107};
simdScore pieceCoordination[Position::lastBitboard] = {
	{0},
	{0},
	{122,41,17,-60},
	{326,-19,-39,-112},
	{498,260,-25,-32},
	{346,213,10,-97}
};

simdScore piecesCenterControl[Position::lastBitboard] = {
	{0},
	{0},
	{80,-53,246,-230},
	{141,-16,180,-183},
	{364,28,88,-170},
	{83,55,70,-74}
};

simdScore piecesBigCenterControl[Position::lastBitboard] = {
	{0},
	{0},
	{-69,-128,-288,-597},
	{-60,2,-302,-538},
	{103,-28,-172,-551},
	{330,29,-186,-541}
};

simdScore rookOn7Bonus =  {3269,1585,-19,43};
simdScore rookOnPawns =  {-1097,704,245,248};
simdScore queenOn7Bonus =  {-3735,6377,-205,-83};
simdScore queenOnPawns =  {-1529,2358,-85,-169};
simdScore rookOnOpen =  {2420,754,-36,17};
simdScore rookOnSemi =  {410,1256,10,77};
simdScore rookTrapped =  {84,614,-199,5};
simdScore rookTrappedKingWithoutCastling =  {965,494,-66,196};
simdScore knightOnOutpost =  {586,-105,137,278};
simdScore knightOnOutpostSupported =  {321,714,-247,-134};
simdScore knightOnHole =  {-4,-16,216,-135};
simdScore KnightAttackingWeakPawn =  {202,20,62,-57};
simdScore bishopOnOutpost =  {-1364,743,-28,-51};
simdScore bishopOnOutpostSupported =  {3317,-126,14,155};
simdScore bishopOnHole =  {934,-631,1,-154};
simdScore badBishop =  {81,1427,-29,-61};
simdScore tempo =  {729,792,-167,162};
simdScore bishopPair =  {3434,4686,142,-51};
simdScore ownKingNearPassedPawn =  {7,91,-83,-67};
simdScore enemyKingNearPassedPawn =  {24,185,38,9};
simdScore spaceBonus =  {309,5,60,233};
simdScore undefendedMinorPenalty =  {680,212,-53,-133};

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	{0,0,0,0},
	{0,0,0,0},//king
	{913,-7654,-152,15},
	{2217,1699,61,-109},
	{3401,426,-56,4},
	{2998,2802,7,8},
	{0,0,0,0},//pawn
	{0,0,0,0},
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//king
	{{0,0,0,0},{0,0,0,0},	{-594,298,51,46},	{3582,282,-199,10},{3614,411,89,-57},{1391,-4392,11,106},{1434,2615,-16,199},	{0,0,0,0}},//queen
	{{0,0,0,0},{0,0,0,0},	{1115,3243,119,-77},	{3124,2379,-98,126},{4854,3246,59,-128},{3181,1693,21,48},{1025,107,29,-67},	{0,0,0,0}},//rook
	{{0,0,0,0},{0,0,0,0},	{2323,4210,-66,-209},	{2590,1903,-182,-6},{359,2675,-67,-44},{3189,4426,177,-56},{-968,4845,-243,-64},	{0,0,0,0}},//bishop
	{{0,0,0,0},{0,0,0,0},	{1212,398,72,-86},	{3139,3022,22,-41},{1880,2830,-11,0},{2333,1112,223,-122},{-755,653,-26,75},	{0,0,0,0}},//knight
	{{0,0,0,0},{0,0,0,0},	{324,-87,87,118},		{76,217,68,222},	{124,160,-178,-79},	{260,127,-66,-201}, {221,205,20,26},	{0,0,0,0}},//pawn
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}}
//						king				queen						rook					bishop					knight					pawn
};
simdScore weakPawnAttackedByKing =  {-65,4956,3,23};
//------------------------------------------------
//king safety 
//------------------------------------------------
simdScore KingAttackWeights =  {415,214,164,174};
simdScore kingShieldBonus =  {2676,101,-143,0};
simdScore kingFarShieldBonus =  {1667,17,-181,-111};
simdScore kingStormBonus =  {247,64,234,-169};
simdScore kingSafetyBonus =  {1800,550,6,-77};
simdScore kingSafetyPars1 =  {646,589,-4,328};
simdScore kingSafetyPars2 =  {1268,959,894,1290};

//------------------------------------------------




simdScore queenVsRook2MinorsImbalance =  {19932,20041,-8,31};

simdScore mobilityBonus[Position::separationBitmap][32] = {
	{0},
	{0},
	{ {-1625,-1500}, {-875,-625}, {125,333}, {125,750}, {583,1416},
	  {916,2250}, {1166,2541}, {1708,3041}, {1791,3291}, {2000,3833},
	  {2333,3916}, {2500,4333}, {2500,4708}, {2750,5000}, {2791,5125},
	  {2916,5250}, {2958,5541}, {3041,5666}, {3291,5833}, {3666,5958},
	  {3666,6166}, {4125,6916}, {4250,7083}, {4250,7291}, {4416,7666},
	  {4541,7958}, {4708,8583}, {4833,8833}
	},
	{ {-2339,-3190},{-1080,-774},{-650,1161},{-463,2308},{-152,2856},
	  {-18,3394},{360,4638},{688,4867},{1168,5443},{1188,5925},
	  {1271,6431},{1533,6880},{1959,6884},{2006,7008},{2390,7111}
	},
	{ {-1860,-2470},{-877,-1070},{278,-513},{888,259},{1292,749},
	  {1705,1083},{2045,1566},{2212,1626},{2486,2124},{2610,2234},
	  {3365,2595},{3187,2851},{3577,3464},{4179,3502}
	},
	{ {-2774, -3298},{-999, -2100},{260,-1003},{657,59},{1116,571},
	  {1551,909},{2079,1010},{1830,1234},{1793,603}
	}
};