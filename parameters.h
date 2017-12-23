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

#include "vajolet.h"
#include "position.h"
#include "tables.h"

extern simdScore queenMobilityPars;
extern simdScore rookMobilityPars;
extern simdScore bishopMobilityPars;
extern simdScore knightMobilityPars;

extern simdScore isolatedPawnPenalty;
extern simdScore isolatedPawnPenaltyOpp;
extern simdScore doubledPawnPenalty;
extern simdScore backwardPawnPenalty;
extern simdScore chainedPawnBonus;
extern simdScore passedPawnFileAHPenalty;
extern simdScore passedPawnSupportedBonus;
extern simdScore candidateBonus;
extern simdScore passedPawnBonus;
extern simdScore passedPawnUnsafeSquares;
extern simdScore passedPawnBlockedSquares;
extern simdScore passedPawnDefendedSquares;
extern simdScore passedPawnDefendedBlockingSquare;
extern simdScore unstoppablePassed;
extern simdScore rookBehindPassedPawn;
extern simdScore EnemyRookBehindPassedPawn;

extern simdScore holesPenalty;
extern simdScore pawnCenterControl;
extern simdScore pawnBigCenterControl;


extern simdScore pieceCoordination;

extern simdScore piecesCenterControl;
extern simdScore piecesBigCenterControl;

extern simdScore rookOn7Bonus;
extern simdScore rookOnPawns;
extern simdScore queenOn7Bonus;
extern simdScore queenOnPawns;
extern simdScore rookOnOpen;
extern simdScore rookOnSemi;
extern simdScore rookTrapped;
extern simdScore rookTrappedKingWithoutCastling;

extern simdScore knightOnOutpost;
extern simdScore knightOnOutpostSupported;
extern simdScore knightOnHole;
extern simdScore KnightAttackingWeakPawn;

extern simdScore bishopOnOutpost;
extern simdScore bishopOnOutpostSupported;
extern simdScore bishopOnHole;
extern simdScore badBishop;

extern simdScore tempo;
extern simdScore bishopPair;

extern simdScore ownKingNearPassedPawn;
extern simdScore enemyKingNearPassedPawn;

extern simdScore spaceBonus;
extern simdScore undefendedMinorPenalty;


extern simdScore attackedByPawnPenalty[Position::separationBitmap];

extern simdScore weakPiecePenalty[Position::separationBitmap][Position::separationBitmap];
extern simdScore weakPawnAttackedByKing;
//------------------------------------------------
//king safety
//------------------------------------------------
extern const unsigned int KingAttackWeights[];
extern simdScore kingShieldBonus;
extern simdScore kingFarShieldBonus;
extern simdScore kingStormBonus;
extern simdScore kingSafetyBonus;
extern simdScore kingSafetyScaling;
extern simdScore KingSafetyMaxAttack;
extern simdScore KingSafetyLinearCoefficent;
extern simdScore KingSafetyMaxResult;
//------------------------------------------------


extern simdScore queenVsRook2MinorsImbalance;





#endif /* EVAL_H_ */