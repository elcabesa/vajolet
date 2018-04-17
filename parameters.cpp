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
		{136902,100000,-19,53},
		{51875,61034,238,-13},
		{35137,35964,128,-30},
		{34332,34818,-210,58},
		{5745,9893,-97,-28},
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

simdScore PawnD3 =  {1543,4,-229,-9};
simdScore PawnD4 =  {1900,-57,26,-135};
simdScore PawnD5 =  {89,43,-182,110};
simdScore PawnE3 =  {349,70,86,60};
simdScore PawnE4 =  {759,-27,11,6};
simdScore PawnE5 =  {-266,47,172,-42};
simdScore PawnCentering =  {-15,-18,1,69};
simdScore PawnRankBonus =  {642,157,-66,-130};
simdScore KnightPST =  {791,543,-75,68};
simdScore BishopPST =  {140,401,-27,27};
simdScore RookPST =  {325,-99,-73,-168};
simdScore QueenPST =  {-113,355,36,-19};
simdScore KingPST =  {861,898,19,-75};

simdScore BishopBackRankOpening =  {315,-51,-188,10};
simdScore KnightBackRankOpening =  {-673,-150,28,50};
simdScore RookBackRankOpening =  {-426,372,23,-1};
simdScore QueenBackRankOpening =  {143,3806,-111,79};
simdScore BishopOnBigDiagonals =  {1166,416,-72,193};



simdScore queenMobilityPars =  {526,439,123,127};
simdScore rookMobilityPars =  {942,1155,509,318};
simdScore bishopMobilityPars =  {-79,346,580,250};
simdScore knightMobilityPars =  {-92,496,545,596};
simdScore isolatedPawnPenalty =  {775,1710,-126,111};
simdScore isolatedPawnPenaltyOpp =  {208,675,-81,189};
simdScore doubledPawnPenalty =  {119,1084,-47,97};
simdScore backwardPawnPenalty =  {1255,1593,-29,59};
simdScore chainedPawnBonus =  {540,79,-53,-52};
simdScore passedPawnFileAHPenalty =  {-1110,766,-70,5};
simdScore passedPawnSupportedBonus =  {1032,-45,-229,-60};
simdScore candidateBonus =  {135,756,78,87};
simdScore passedPawnBonus =  {386,295,-49,102};
simdScore passedPawnUnsafeSquares =  {-261,173,-148,-33};
simdScore passedPawnBlockedSquares =  {-62,156,-27,-136};
simdScore passedPawnDefendedSquares =  {-200,175,-221,-227};
simdScore passedPawnDefendedBlockingSquare =  {91,224,97,48};
simdScore unstoppablePassed =  {-85,1787,-138,13};
simdScore rookBehindPassedPawn =  {193,527,-114,181};
simdScore EnemyRookBehindPassedPawn =  {12,267,107,-78};
simdScore holesPenalty =  {217,7,120,-65};
simdScore pawnCenterControl =  {251,-400,41,33};
simdScore pawnBigCenterControl =  {896,-287,7,-18};
simdScore pieceCoordination =  {494,184,-235,-13};
simdScore piecesCenterControl =  {172,63,216,-92};
simdScore piecesBigCenterControl =  {133,-81,-76,-142};
simdScore rookOn7Bonus =  {3185,1780,18,96};
simdScore rookOnPawns =  {-1123,661,231,143};
simdScore queenOn7Bonus =  {-3733,6378,-211,-69};
simdScore queenOnPawns =  {-1568,2343,-81,-176};
simdScore rookOnOpen =  {2617,880,-15,44};
simdScore rookOnSemi =  {512,1593,-2,173};
simdScore rookTrapped =  {-130,412,-126,-17};
simdScore rookTrappedKingWithoutCastling =  {760,487,58,117};
simdScore knightOnOutpost =  {569,-191,-49,219};
simdScore knightOnOutpostSupported =  {61,1204,-90,-115};
simdScore knightOnHole =  {1341,1441,130,6};
simdScore KnightAttackingWeakPawn =  {302,536,-72,-86};
simdScore bishopOnOutpost =  {-1379,755,14,-61};
simdScore bishopOnOutpostSupported =  {3326,-101,-45,152};
simdScore bishopOnHole =  {903,-682,4,-176};
simdScore badBishop =  {86,1428,-84,-15};
simdScore tempo =  {625,487,-23,106};
simdScore bishopPair =  {3661,4759,90,42};
simdScore ownKingNearPassedPawn =  {-19,57,-180,12};
simdScore enemyKingNearPassedPawn =  {-41,228,-4,-54};
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
simdScore kingSafetyBonus =  {1800,55,6,-77};
simdScore kingSafetyPars1 =  {646,589,-4,328};
simdScore kingSafetyPars2 =  {1268,959,894,1290};

//------------------------------------------------




simdScore queenVsRook2MinorsImbalance={20000,20000,0,0};
