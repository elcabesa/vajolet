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
simdScore spaceBonus =  {215,-31,0,0};
simdScore undefendedMinorPenalty =  {51,68,0,0};

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	{0,0,0,0},
	{0,0,0,0},//king
	{9397,2233,0,0},
	{5773,6132,0,0},
	{6523,3960,0,0},
	{5690,6268,0,0},
	{0,0,0,0},//pawn
	{0,0,0,0},
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//king
	{{0,0,0,0},{0,0,0,0},	{-139,358,0,0},		{6033,2881,0,0},{5604,3529,0,0},{4944,3186,0,0},{888,-338,0,0},	{0,0,0,0}},//queen
	{{0,0,0,0},{0,0,0,0},	{-2060,1678,0,0},	{-1576,2598,0,0},{3783,5690,0,0},{4343,5341,0,0},{490,103,0,0},	{0,0,0,0}},//rook
	{{0,0,0,0},{0,0,0,0},	{-767,150,0,0},		{1158,3371,0,0},{-98,-35,0,0},	{2248,1069,0,0},{412,-70,0,0},	{0,0,0,0}},//bishop
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//knight
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//pawn
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
