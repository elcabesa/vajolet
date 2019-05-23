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
#include "movepicker.h"
#include "position.h"
#include "rootMove.h"
#include "tbCommonData.h"
#include "syzygy.h"

Syzygy::Syzygy() {
	TBCommonData::init();
}

void Syzygy::setPath(const std::string s) {
	TBFile::setPaths(s); 
	_t.clear();
	_t.init();
}

size_t Syzygy::getSize() const {
	return _t.size();
}

// For a position where the side to move has a winning capture it is not necessary
// to store a winning value so the generator treats such positions as "don't cares"
// and tries to assign to it a value that improves the compression ratio. Similarly,
// if the side to move has a drawing capture, then the position is at least drawn.
// If the position is won, then the TB needs to store a win value. But if the
// position is drawn, the TB may store a loss value if that is better for compression.
// All of this means that during probing, the engine must look at captures and probe
// their results and must probe the position itself. The "best" result of these
// probes is the correct result for the position.
// DTZ tables do not store values when a following move is a zeroing winning move
// (winning capture or winning pawn move). Also DTZ store wrong values for positions
// where the best move is an ep-move (even if losing). So in all these cases set
// the state to ZEROING_BEST_MOVE.
WDLScore Syzygy::_search(Position& pos, ProbeState& result, const bool CheckZeroingMoves) const {
	
	WDLScore value, bestValue = WDLLoss;
	size_t totalCount = pos.getNumberOfLegalMoves(), moveCount = 0;
	
	Move m;
	MovePicker mp(pos);
	while((m = mp.getNextMove())) {
		if ( !pos.isCaptureMove(m)
				&& (!CheckZeroingMoves || !isPawn(pos.getPieceAt(m.getFrom())))) {
			continue;
		}

		moveCount++;

		pos.doMove(m);
		value = -_search(pos, result, false);
		pos.undoMove();

		if (result == FAIL) {
				return WDLDraw;
		}

		if (value > bestValue) {
			bestValue = value;

			if (value >= WDLWin) {
					result = ZEROING_BEST_MOVE; // Winning DTZ-zeroing move
					return value;
			}
		}
	}

	// In case we have already searched all the legal moves we don't have to probe
	// the TB because the stored score could be wrong. For instance TB tables
	// do not contain information on position with ep rights, so in this case
	// the result of probe_wdl_table is wrong. Also in case of only capture
	// moves, for instance here 4K3/4q3/6p1/2k5/6p1/8/8/8 w - - 0 7, we have to
	// return with ZEROING_BEST_MOVE set.
	bool noMoreMoves = (moveCount && moveCount == totalCount);

	if (noMoreMoves) {
		value = bestValue;
	} else {
		value = (WDLScore)_t.probeWDL(pos, result);

		if (result == FAIL) {
			return WDLDraw;
		}
	}

	// DTZ stores a "don't care" value if bestValue is a win
	if (bestValue >= value) {
		return result = (   bestValue > WDLDraw || noMoreMoves ? ZEROING_BEST_MOVE : OK), bestValue;
	}
	return result = OK, value;
}

// Probe the WDL table for a particular position.
// If *result != FAIL, the probe was successful.
// The return value is from the point of view of the side to move:
// -2 : loss
// -1 : loss, but draw under 50-move rule
//  0 : draw
//  1 : win, but draw under 50-move rule
//  2 : win
WDLScore Syzygy::probeWdl(Position& pos, ProbeState& result) const{

    result = OK;
    return _search(pos, result, false);
}

size_t Syzygy::getMaxCardinality() const {
	return _t.getMaxCardinality();
}



// DTZ tables don't store valid scores for moves that reset the rule50 counter
// like captures and pawn moves but we can easily recover the correct dtz of the
// previous move if we know the position's WDL score.
int Syzygy::_dtzBeforeZeroing(WDLScore wdl) {
	return 
		wdl == WDLWin         ?  1   :
		wdl == WDLCursedWin   ?  101 :
		wdl == WDLBlessedLoss ? -101 :
		wdl == WDLLoss        ? -1   : 0;
}

// Return the sign of a number (-1, 0, 1)
int Syzygy::_signOf(int val) {
    return (0 < val) - (val < 0);
}

// Probe the DTZ table for a particular position.
// If *result != FAIL, the probe was successful.
// The return value is from the point of view of the side to move:
//         n < -100 : loss, but draw under 50-move rule
// -100 <= n < -1   : loss in n ply (assuming 50-move counter == 0)
//        -1        : loss, the side to move is mated
//         0        : draw
//     1 < n <= 100 : win in n ply (assuming 50-move counter == 0)
//   100 < n        : win, but draw under 50-move rule
//
// The return value n can be off by 1: a return value -n can mean a loss
// in n+1 ply and a return value +n can mean a win in n+1 ply. This
// cannot happen for tables with positions exactly on the "edge" of
// the 50-move rule.
//
// This implies that if dtz > 0 is returned, the position is certainly
// a win if dtz + 50-move-counter <= 99. Care must be taken that the engine
// picks moves that preserve dtz + 50-move-counter <= 99.
//
// If n = 100 immediately after a capture or pawn move, then the position
// is also certainly a win, and during the whole phase until the next
// capture or pawn move, the inequality to be preserved is
// dtz + 50-movecounter <= 100.
//
// In short, if a move is available resulting in dtz + 50-move-counter <= 99,
// then do not accept moves leading to dtz + 50-move-counter == 100.
int Syzygy::probeDtz(Position& pos, ProbeState& result) const {

	result = OK;
	WDLScore wdl = _search(pos, result, true);

	if (result == FAIL || wdl == WDLDraw) {// DTZ tables don't store draws
		return 0;
	}

	// DTZ stores a 'don't care' value in this case, or even a plain wrong
	// one as in case the best move is a losing ep, so it cannot be probed.
	if (result == ZEROING_BEST_MOVE) {
		return _dtzBeforeZeroing(wdl);
	}

	int dtz = _t.probeDTZ(pos, result, wdl);

	if (result == FAIL) {
		return 0;
	}

	if (result != CHANGE_STM) {
		return (dtz + 100 * (wdl == WDLBlessedLoss || wdl == WDLCursedWin)) * _signOf(wdl);
	}

	// DTZ stores results for the other side, so we need to do a 1-ply search and
	// find the winning move that minimizes DTZ.
	int minDTZ = 0xFFFF;

	Move m;
	MovePicker mp(pos);
	while((m = mp.getNextMove())) {
		bool zeroing = pos.isCaptureMove(m) || isPawn(pos.getPieceAt(m.getFrom()));
		
		pos.doMove(m);

		// For zeroing moves we want the dtz of the move _before_ doing it,
		// otherwise we will get the dtz of the next move sequence. Search the
		// position after the move to get the score sign (because even in a
		// winning position we could make a losing capture or going for a draw).
		dtz = zeroing 
			? -_dtzBeforeZeroing(_search(pos, result, false))
			: -probeDtz(pos, result);

		// If the move mates, force minDTZ to 1
		if (dtz == 1 && pos.isInCheck() && pos.getNumberOfLegalMoves() == 0) {
			minDTZ = 1;
		}

		// Convert result from 1-ply search. Zeroing moves are already accounted
		// by dtz_before_zeroing() that returns the DTZ of the previous move.
		if (!zeroing) {
			dtz += _signOf(dtz);
		}

		// Skip the draws and if we are winning only pick positive dtz
		if (dtz < minDTZ && _signOf(dtz) == _signOf(wdl)) {
			minDTZ = dtz;
		}

		pos.undoMove();

		if (result == FAIL) {
			return 0;
		}
	}

	// When there are no legal moves, the position is mate: we return -1
	return minDTZ == 0xFFFF ? -1 : minDTZ;
}

// Use the DTZ tables to rank root moves.
//
// A return value false indicates that not all probes were successful.
bool Syzygy::_rootProbe(Position& pos, std::vector<Move>& rootMoves, bool Syzygy50MoveRule) const {
	/*ProbeState result;

	// Obtain 50-move counter for the root position
	int cnt50 = pos.getActualState().getIrreversibleMoveCount();

	// Check whether a position was repeated since the last zeroing move.
	bool rep = pos.hasRepeated(true);

	int dtz, bound = Syzygy50MoveRule ? 900 : 1;

	// Probe and rank each move
	for (auto& m : rootMoves)
	{
			pos.doMove(m);

			// Calculate dtz for the current move counting from the root position
			if (pos.getActualState().getIrreversibleMoveCount() == 0)
			{
					// In case of a zeroing move, dtz is one of -101/-1/0/1/101
					WDLScore wdl = (WDLScore)-_t.probeWDL(pos, result);
					dtz = _dtzBeforeZeroing(wdl);
			}
			else
			{
					// Otherwise, take dtz for the new position and correct by 1 ply
					dtz = -_t.probeDTZ(pos, result);
					dtz =  dtz > 0 ? dtz + 1
							 : dtz < 0 ? dtz - 1 : dtz;
			}

			// Make sure that a mating move is assigned a dtz value of 1
			if (pos.isInCheck() && dtz == 2 && pos.getNumberOfLegalMoves() == 0) {
				dtz = 1;
			}

			pos.undoMove();

			if (result == FAIL)
					return false;

			// Better moves are ranked higher. Certain wins are ranked equally.
			// Losing moves are ranked equally unless a 50-move draw is in sight.
			int r =  dtz > 0 ? (dtz + cnt50 <= 99 && !rep ? 1000 : 1000 - (dtz + cnt50))
						 : dtz < 0 ? (-dtz * 2 + cnt50 < 100 ? -1000 : -1000 + (-dtz + cnt50))
						 : 0;
			m.tbRank = r;

			// Determine the score to be displayed for this move. Assign at least
			// 1 cp to cursed wins and let it grow to 49 cp as the positions gets
			// closer to a real win.
			m.tbScore =  r >= bound ? VALUE_MATE - MAX_PLY - 1
								 : r >  0     ? Value((std::max( 3, r - 800) * int(PawnValueEg)) / 200)
								 : r == 0     ? VALUE_DRAW
								 : r > -bound ? Value((std::min(-3, r + 800) * int(PawnValueEg)) / 200)
								 :             -VALUE_MATE + MAX_PLY + 1;
	}
*/
	return true;
}

// Use the WDL tables to rank root moves.
// This is a fallback for the case that some or all DTZ tables are missing.
//
// A return value false indicates that not all probes were successful.
bool Syzygy::_rootProbeWdl(Position& pos, std::vector<Move>& rootMoves, bool Syzygy50MoveRule) const {
	/*static const int WDLToRank[] = { -1000, -899, 0, 899, 1000 };
	ProbeState result;
	
	// Probe and rank each move
	for (auto& m : rootMoves)
	{
			pos.doMove(m);

			WDLScore wdl = (WDLScore)-_t.probeWDL(pos, result);

			pos.undoMove();

			if (result == FAIL)
					return false;

			m.tbRank = WDLToRank[wdl + 2];

			if (!Syzygy50MoveRule)
					wdl =  wdl > WDLDraw ? WDLWin
							 : wdl < WDLDraw ? WDLLoss : WDLDraw;
			m.tbScore = _WDLToValue(wdl + 2);
	}
*/
	return true;
}

int Syzygy::_WDLToValue(int value) {
	/*assert(value >= -2 && value <= 2);
	const int retValues[] = {
	-VALUE_MATE + MAX_PLY + 1,
	VALUE_DRAW - 2,
	VALUE_DRAW,
	VALUE_DRAW + 2,
	VALUE_MATE - MAX_PLY - 1};
	
	return retValues[value + 2];*/
	return 0;
}