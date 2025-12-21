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



#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "bitBoardIndex.h"
#include "score.h"
#include "tSquare.h"


class EvalParameters
{
public:
	EvalParameters();
	
	simdScore initialPieceValue[lastBitboard] = {
		{0,0,0,0},
		{3000000,3000000,0,0},//king
		{118976,105579,0,0},
		{52614,55708,0,0},
		{33773,37122,0,0},
		{33320,36292,0,0},
		{7759,9770,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0}};
	
	simdScore PawnD3 =  {0,0,0,0};
	simdScore PawnD4 =  {0,0,0,0};
	simdScore PawnD5 =  {0,0,0,0};
	simdScore PawnE3 =  {0,0,0,0};
	simdScore PawnE4 =  {0,0,0,0};
	simdScore PawnE5 =  {0,0,0,0};
	simdScore PawnCentering =  {274,-28,-445,1};
	simdScore PawnRankBonus =  {165,357,1,-396};
	simdScore KnightPST =  {938,806,-204,70};
	simdScore BishopPST =  {142,329,-669,-39};
	simdScore RookPST =  {209,-30,-129,142};
	simdScore QueenPST =  {184,278,738,95};
	simdScore KingPST =  {1120,1084,47,107};
	
	simdScore BishopBackRankOpening =  {923,338,0,0};
	simdScore KnightBackRankOpening =  {2,-200,0,0};
	simdScore RookBackRankOpening =  {356,320,0,0};
	simdScore QueenBackRankOpening =  {-440,3290,0,0};
	simdScore BishopOnBigDiagonals =  {563,106,0,0};

	simdScore isolatedPawnPenalty =  {695,1284,0,0};
	simdScore isolatedPawnPenaltyOpp =  {809,1107,0,0};
	simdScore doubledPawnPenalty =  {536,766,0,0};
	simdScore backwardPawnPenalty =  {1024,765,0,0};
	simdScore chainedPawnBonus =  {55,59,0,0};
	simdScore chainedPawnBonusOffset =  {296,1,0,0};
	simdScore chainedPawnBonusOpp =  {23,61,0,0};
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
	simdScore holesPenalty =  {19,-166,0,0};
	simdScore pawnCenterControl =  {472,-125,0,0};
	simdScore pawnBigCenterControl =  {61,194,0,0};

	simdScore pieceCoordination[lastBitboard] = {
		{0},
		{0},
		{122,41,0,0},
		{326,-19,0,0},
		{498,260,0,0},
		{346,213,0,0}
	};

	simdScore piecesCenterControl[lastBitboard] = {
		{0},
		{0},
		{80,-53,0,0},
		{141,-16,0,0},
		{364,28,0,0},
		{83,55,0,0}
	};

	simdScore piecesBigCenterControl[lastBitboard] = {
		{0},
		{0},
		{-69,-128,0,0},
		{-60,2,0,0},
		{103,-28,0,0},
		{330,29,0,0}
	};

	simdScore rookOn7Bonus =  {3269,1585,0,0};
	simdScore rookOnPawns =  {-1097,704,0,0};
	simdScore queenOn7Bonus =  {-3735,6377,0,0};
	simdScore queenOnPawns =  {-1529,2358,0,0};
	simdScore rookOnOpen =  {2420,754,0,0};
	simdScore rookOnSemi =  {410,1256,0,0};
	simdScore rookTrapped =  {84,614,0,0};
	simdScore rookTrappedKingWithoutCastling =  {965,494,0,0};
	simdScore knightOnOutpost =  {586,-105,0,0};
	simdScore knightOnOutpostSupported =  {321,714,0,0};
	simdScore knightOnHole =  {-4,-16,0,0};
	simdScore KnightAttackingWeakPawn =  {202,20,0,0};
	simdScore bishopOnOutpost =  {-1364,743,0,0};
	simdScore bishopOnOutpostSupported =  {3317,-126,0,0};
	simdScore bishopOnHole =  {934,-631,0,0};
	simdScore badBishop =  {81,1427,0,0};
	simdScore tempo =  {729,792,0,0};
	simdScore bishopPair =  {3434,4686,0,0};
	simdScore ownKingNearPassedPawn =  {7,91,0,0};
	simdScore enemyKingNearPassedPawn =  {24,185,0,0};
	simdScore spaceBonus =  {215,-31,0,0};
	simdScore undefendedMinorPenalty =  {259,234,0,0};

	simdScore attackedByPawnPenalty[separationBitmap]=
	{	{0,0,0,0},
		{0,0,0,0},//king
		{8509,3188,0,0},
		{6007,6689,0,0},
		{6570,5035,0,0},
		{5975,6359,0,0},
		{0,0,0,0},//pawn
		{0,0,0,0},
	};

	simdScore weakPiecePenalty[separationBitmap][separationBitmap]=
	{	{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},
		{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}},//king
		{{0,0,0,0},{0,0,0,0},	{-139,358,0,0},		{6033,2881,0,0},{5604,3529,0,0},{4944,3186,0,0},{1414,-578,0,0},{0,0,0,0}},//queen
		{{0,0,0,0},{0,0,0,0},	{-2060,1678,0,0},	{-1576,2598,0,0},{3783,5690,0,0},{4343,5341,0,0},{482,123,0,0},	{0,0,0,0}},//rook
		{{0,0,0,0},{0,0,0,0},	{-1207,809,0,0},	{954,3657,0,0},	{-220,263,0,0},	{2786,2409,0,0},{241,368,0,0},	{0,0,0,0}},//bishop
		{{0,0,0,0},{0,0,0,0},	{-1073,1197,0,0},	{954,4394,0,0},	{2472,2746,0,0},{-699,1027,0,0},{83,846,0,0},	{0,0,0,0}},//knight
		{{0,0,0,0},{0,0,0,0},	{-452,748,0,0},		{-148,2078,0,0},{-197,1644,0,0},{-990,1691,0,0},{297,1901,0,0},	{0,0,0,0}},//pawn
		{{0,0,0,0},{0,0,0,0},	{0,0,0,0},			{0,0,0,0},		{0,0,0,0},		{0,0,0,0},		{0,0,0,0},	{0,0,0,0}}
	//						king				queen						rook					bishop					knight					pawn
	};
	simdScore weakPawnAttackedByKing =  {3144,4302,0,0};
	//------------------------------------------------
	//king safety 
	//------------------------------------------------
	simdScore KingAttackWeights =  {29,58,46,75};
	simdScore kingShieldBonus =  {2709,0,0,0};
	simdScore kingFarShieldBonus =  {1951,0,0,0};
	simdScore kingStormBonus =  {500,200,217,0};
	simdScore kingSafetyBonus =  {84,3,0,0};
	simdScore kingSafetyPars1 =  {56,179,-13,870};
	simdScore kingSafetyPars2 =  {780,880,435,790};

	//------------------------------------------------




	simdScore queenVsRook2MinorsImbalance =  {19932,20041,0,0};

	simdScore mobilityBonus[separationBitmap][32] = {
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


	simdScore _pstValue[lastBitboard][squareNumber];
	simdScore _nonPawnValue[lastBitboard];
	simdScore _pieceValue[lastBitboard];
	
	void updateValues();

private:
	void _initValues();
	void _initPstValues();

};

class SearchParameters
{
public:
	SearchParameters();
	int razorMargin;
	int razorMarginDepth;
	int razorMarginCut;
	int razorDepth;
	bool razorReturn;

	int staticNullMovePruningDepth;
	int staticNullMovePruningValue;
	int staticNullMovePruningImprovingBonus;

	int nullMovePruningDepth;
	int nullMovePruningReduction;
	float nullMovePruningBonusDepth;
	int nullMovePruningBonusThreshold;
	int nullMovePruningBonusAdditionalRed;
	int nullMovePruningVerificationDepth;
	
	int probCutDepth;
	int probCutDelta;
	int probCutDepthRed;
	
	int iidDepthPv;
	int iidDepthNonPv;
	int iidStaticEvalBonus;
	int iidDepthRed;
	int iidDepthRedFactor;
	
	int singularExtensionPVDepth;
	int singularExtensionNonPVDepth;
	int singularExtensionTtDepth;
	int singularExtensionScoreDepthBonus;
	int singularExtensionExt;

	int dangerousMoveExtension;
	int checkMoveExtension;

	int FutilityMoveCountsDepth;
	unsigned int FutilityMoveCounts[2][32];

	int futilityDepth;
	Score futilityMargin[12];// todo change with a not scaled depth formula

	int negativeSeeSkipDepth;

	int lmrDepthLimitInf;

	static constexpr unsigned int LmrLimit = 32;
	static constexpr unsigned int LmrLimitMove = 64;
	Score PVreduction[2][LmrLimit * 16][LmrLimitMove];
	Score nonPVreduction[2][LmrLimit * 16][LmrLimitMove];
private:
	void _updatePars();
};

#endif /* EVAL_H_ */
