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
		{137000,100000,0,0},//queen
		{52000,61000,0,0},//rook
		{35300,36100,0,0},//bishop
		{34500,34900,0,0},//knight
		{5700,10000,0,0},//panws
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

simdScore PawnD3 = {1755,0,0,0};
simdScore PawnD4 = {2100,0,0,0};
simdScore PawnD5 = {85,0,0,0};
simdScore PawnE3 = {185,0,0,0};
simdScore PawnE4 = {620,0,0,0};
simdScore PawnE5 = {-5,0,0,0};
simdScore PawnCentering = {141,-119,0,0};
simdScore PawnRankBonus = {450,30,0,0};
simdScore KnightPST = {545,462,0,0};
simdScore BishopPST = {22,273,0,0};
simdScore RookPST = {418,-290,0,0};
simdScore QueenPST = {-170,342,0,0};
simdScore KingPST = {700,787,0,0};

simdScore BishopBackRankOpening = {400,-200,0,0};
simdScore KnightBackRankOpening = {-800,-300,0,0};
simdScore RookBackRankOpening = {-400,400,0,0};
simdScore QueenBackRankOpening = {200,3900,0,0};
simdScore BishopOnBigDiagonals = {1400,600,0,0};



simdScore queenMobilityPars={410,361,100,554};
simdScore rookMobilityPars={805,627,376,438};
simdScore bishopMobilityPars={-105,172,639,299};
simdScore knightMobilityPars={-166,380,674,577};
simdScore isolatedPawnPenalty={749,1598,0,0};
simdScore isolatedPawnPenaltyOpp={216,597,0,0};
simdScore doubledPawnPenalty={45,1241,0,0};
simdScore backwardPawnPenalty={1305,1544,0,0};
simdScore chainedPawnBonus={721,26,0,0};
simdScore passedPawnFileAHPenalty = {-1065,580,0,0};
simdScore passedPawnSupportedBonus = {836,-338,0,0};
simdScore candidateBonus = {-136,872,0,0};
simdScore passedPawnBonus = {263,662,0,0};
simdScore passedPawnUnsafeSquares ={-195,369,0,0};
simdScore passedPawnBlockedSquares ={-112,123,0,0};
simdScore passedPawnDefendedSquares = {-247,208,0,0};
simdScore passedPawnDefendedBlockingSquare = {34,259,0,0};
simdScore unstoppablePassed = {0,2287,0,0};
simdScore rookBehindPassedPawn = {169,510,0,0};
simdScore EnemyRookBehindPassedPawn = {1,453,0,0};
simdScore holesPenalty={171,66,0,0};
simdScore pawnCenterControl={179,-375,0,0};
simdScore pawnBigCenterControl={822,-265,0,0};
simdScore pieceCoordination={603,423,0,0};
simdScore piecesCenterControl={140,181,0,0};
simdScore piecesBigCenterControl={43,213,0,0};
simdScore rookOn7Bonus={3232,1834,0,0};
simdScore rookOnPawns={-1036,888,0,0};
simdScore queenOn7Bonus={-3678,6515,0,0};
simdScore queenOnPawns={-1475,2518,0,0};
simdScore rookOnOpen={2485,919,0,0};
simdScore rookOnSemi={488,1477,0,0};
simdScore rookTrapped = {-109,482,0,0};
simdScore rookTrappedKingWithoutCastling = {787,478,0,0};
simdScore knightOnOutpost= {403,-123,0,0};
simdScore knightOnOutpostSupported= {15,1272,0,0};
simdScore knightOnHole= {1440,1353,0,0};
simdScore KnightAttackingWeakPawn= {131,473,0,0};
simdScore bishopOnOutpost= {-1198,637,0,0};
simdScore bishopOnOutpostSupported= {3441,114,0,0};
simdScore bishopOnHole= {765,-549,0,0};
simdScore badBishop= {-19,1445,0,0};
simdScore tempo= {707,657,0,0};
simdScore bishopPair ={3409,4863,0,0};
simdScore ownKingNearPassedPawn={-44,114,0,0};
simdScore enemyKingNearPassedPawn={-86,251,0,0};
simdScore spaceBonus={439,135,0,0};
simdScore undefendedMinorPenalty = {869,287,0,0};

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	{0,0,0,0},
	{0,0,0,0},//king
	{1013,-7786,0,0},//queen
	{2238,1614,0,0},//rook
	{3213,363,0,0},//bishop
	{2913,2736,0,0},//knight
	{0,0,0,0},//pawn
	{0,0,0,0},
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//king
	{{0,0,0,0},{0,0,0,0},	{-473,286,0,0},	{3825,134,0,0},{3593,613,0,0},{1433,-4389,0,0},{1413,2713,0,0},	{0,0,0,0}},//queen
	{{0,0,0,0},{0,0,0,0},	{1186,3186,0,0},	{3206,2406,0,0},{4886,3086,0,0},{3413,1686,0,0},{1013,86,0,0},	{0,0,0,0}},//rook
	{{0,0,0,0},{0,0,0,0},	{2386,3986,0,0},	{2833,1863,0,0},{386,2787,0,0},{3096,4386,0,0},{-886,4702,0,0},	{0,0,0,0}},//bishop
	{{0,0,0,0},{0,0,0,0},	{1286,388,0,0},	{3108,3133,0,0},{1733,2930,0,0},{2209,1103,0,0},{-796,540,0,0},	{0,0,0,0}},//knight
	{{0,0,0,0},{0,0,0,0},	{36,26,0,0},		{69,63,0,0},	{87,93,0,0},	{136,103,0,0},	{186,182,0,0},	{0,0,0,0}},//pawn
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}}
//						king				queen						rook					bishop					knight					pawn
};
simdScore weakPawnAttackedByKing={86,4986,0,0};
//------------------------------------------------
//king safety
//------------------------------------------------
simdScore KingAttackWeights =  {500,300,200,200};
simdScore kingShieldBonus =  {2400,0,0,0};
simdScore kingFarShieldBonus =  {1800,0,0,0};
simdScore kingStormBonus =  {150,10,0,0};
simdScore kingSafetyBonus =  {2000,100,0,0};
simdScore kingSafetyPars1 =  {500,500,10,200};
simdScore kingSafetyPars2 =  {1000,1000,1000,1000};

//------------------------------------------------




simdScore queenVsRook2MinorsImbalance={20000,20000,0,0};
