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