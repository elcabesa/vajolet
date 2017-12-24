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



simdScore queenMobilityPars =  {437,383,111,528};
simdScore rookMobilityPars =  {776,628,372,431};
simdScore bishopMobilityPars =  {-132,187,662,292};
simdScore knightMobilityPars =  {-191,386,700,591};
simdScore isolatedPawnPenalty =  {775,1625,0,0};
simdScore isolatedPawnPenaltyOpp =  {243,623,0,0};
simdScore doubledPawnPenalty =  {18,1214,0,0};
simdScore backwardPawnPenalty =  {1327,1545,0,0};
simdScore chainedPawnBonus =  {735,42,0,0};
simdScore passedPawnFileAHPenalty =  {-1076,556,0,0};
simdScore passedPawnSupportedBonus =  {809,-311,0,0};
simdScore candidateBonus =  {-159,845,0,0};
simdScore passedPawnBonus =  {235,636,0,0};
simdScore passedPawnUnsafeSquares =  {-220,394,0,0};
simdScore passedPawnBlockedSquares =  {-133,101,0,0};
simdScore passedPawnDefendedSquares =  {-270,230,0,0};
simdScore passedPawnDefendedBlockingSquare =  {18,252,0,0};
simdScore unstoppablePassed =  {0,2313,0,0};
simdScore rookBehindPassedPawn =  {182,532,0,0};
simdScore EnemyRookBehindPassedPawn =  {-27,476,0,0};
simdScore holesPenalty =  {159,50,0,0};
simdScore pawnCenterControl =  {207,-400,0,0};
simdScore pawnBigCenterControl =  {847,-252,0,0};
simdScore pieceCoordination =  {618,437,0,0};
simdScore piecesCenterControl =  {117,184,0,0};
simdScore piecesBigCenterControl =  {27,219,0,0};
simdScore rookOn7Bonus =  {3204,1860,0,0};
simdScore rookOnPawns =  {-1061,861,0,0};
simdScore queenOn7Bonus =  {-3704,6488,0,0};
simdScore queenOnPawns =  {-1501,2491,0,0};
simdScore rookOnOpen =  {2511,945,0,0};
simdScore rookOnSemi =  {473,1500,0,0};
simdScore rookTrapped =  {-130,508,0,0};
simdScore rookTrappedKingWithoutCastling =  {813,504,0,0};
simdScore knightOnOutpost =  {399,-145,0,0};
simdScore knightOnOutpostSupported =  {17,1291,0,0};
simdScore knightOnHole =  {1414,1379,0,0};
simdScore KnightAttackingWeakPawn =  {104,499,0,0};
simdScore bishopOnOutpost =  {-1225,610,0,0};
simdScore bishopOnOutpostSupported =  {3416,87,0,0};
simdScore bishopOnHole =  {791,-522,0,0};
simdScore badBishop =  {6,1427,0,0};
simdScore tempo =  {733,683,0,0};
simdScore bishopPair =  {3435,4888,0,0};
simdScore ownKingNearPassedPawn =  {-58,112,0,0};
simdScore enemyKingNearPassedPawn =  {-102,258,0,0};
simdScore spaceBonus =  {412,148,0,0};
simdScore undefendedMinorPenalty =  {893,262,0,0};

simdScore attackedByPawnPenalty[Position::separationBitmap]=
{	{0,0,0,0},
	{0,0,0,0},//king
	{1026,-7773,0,0},//queen
	{2251,1601,0,0},//rook
	{3226,376,0,0},//bishop
	{2926,2724,0,0},//knight
	{0,0,0,0},//pawn
	{0,0,0,0},
};

simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap]=
{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},			{0,0,0,0},		{0,0,0,0}},
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},			{0,0,0,0},		{0,0,0,0}},//king
	{{0,0,0,0},{0,0,0,0},	{-486,273,0,0},		{3838,147,0,0},	{3606,626,0,0},	{1446,-4375,0,0},	{1426,2726,0,0},{0,0,0,0}},//queen
	{{0,0,0,0},{0,0,0,0},	{1173,3173,0,0},	{3193,2393,0,0},{4873,3073,0,0},{3426,1674,0,0},	{1026,73,0,0},	{0,0,0,0}},//rook
	{{0,0,0,0},{0,0,0,0},	{2373,3973,0,0},	{2846,1876,0,0},{373,2773,0,0},	{3109,4373,0,0},	{-873,4716,0,0},{0,0,0,0}},//bishop
	{{0,0,0,0},{0,0,0,0},	{1273,376,0,0},		{3095,3120,0,0},{1729,2943,0,0},{2196,1090,0,0},	{-783,527,0,0},	{0,0,0,0}},//knight
	{{0,0,0,0},{0,0,0,0},	{23,13,0,0},		{66,76,0,0},	{74,106,0,0},	{123,116,0,0},		{173,195,0,0},	{0,0,0,0}},//pawn
	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},			{0,0,0,0},	{0,0,0,0}}
//				king		queen				rook			bishop			knight				pawn
};
simdScore weakPawnAttackedByKing =  {73,4973,0,0};
//------------------------------------------------
//king safety
//------------------------------------------------
const unsigned int KingAttackWeights[] = { 0, 0, 5, 3, 2, 2 };
simdScore kingShieldBonus =  {2404,0,0,0};
simdScore kingFarShieldBonus =  {1624,0,0,0};
simdScore kingStormBonus =  {150,0,0,0};
simdScore kingSafetyBonus =  {92,-5,0,0};
simdScore kingSafetyScaling =  {298,0,0,0};
simdScore KingSafetyMaxAttack =  {112,0,0,0};
simdScore KingSafetyLinearCoefficent =  {507,0,0,0};
simdScore KingAttackUnitWeigth =  {31,84,15,8};
simdScore KingSafetyMaxResult =  {1000,0,0,0};
//------------------------------------------------




simdScore queenVsRook2MinorsImbalance={20000,20000,0,0};
