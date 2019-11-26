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

#include <condition_variable>
#include <mutex>
#include <string>

#include "command.h"
#include "parameters.h"
#include "player.h"
#include "PGNGame.h"
#include "PGNGameResult.h"
#include "PGNMove.h"
#include "PGNMoveList.h"
#include "PGNPly.h"
#include "PGNTag.h"
#include "PGNTagList.h"
#include "selfplay.h"
#include "searchResult.h"
#include "thread.h"
#include "tunerPars.h"
#include "vajo_io.h"

SelfPlay::SelfPlay(const Player& white, const Player& black) : _p(Position::pawnHash::off), _c(TunerParameters::gameTime, TunerParameters::gameTimeIncrement), _white(white), _black(black) {
	_p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	_sl.setWTime(_c.getWhiteTime());
	_sl.setBTime(_c.getBlackTime());
	
	_sl.setWInc(TunerParameters::gameTimeIncrement * 1000);
	_sl.setBInc(TunerParameters::gameTimeIncrement * 1000);
}

pgn::Game SelfPlay::playGame(unsigned int round) {
	// init locks
	std::mutex m;
	std::unique_lock<std::mutex> lock(m);
	
	// init search & time management
	my_thread &thr = my_thread::getInstance();
	thr.setMute(true);
	
	pgn::Game pgnGame;
	_addGameTags(pgnGame, round);
	
	int moveCount = 1;
	_c.start();
	
	pgn::Ply whitePly;
	pgn::Ply blackPly;
	bool pendingMove = false;
	
	while (!_isGameFinished()) {
		// set time limit
		_sl.setWTime(_c.getWhiteTime());
		_sl.setBTime(_c.getBlackTime());
		
		if(_c.isWhiteTurn()) {
			thr.getSearchParameters() = _white.getSearchParametersConst();
		} else {
			thr.getSearchParameters() = _black.getSearchParametersConst();
		}
		
		thr.startThinking(_p, _sl);
		thr.finished().wait(lock);

		auto res = thr.getResult();

		if(_c.isWhiteTurn()) {
			whitePly = pgn::Ply(UciManager::displayMove(_p, res.PV.getMove(0)));
			pendingMove =true;
		}	else {
			blackPly = pgn::Ply(UciManager::displayMove(_p, res.PV.getMove(0)));
			pgnGame.moves().push_back(pgn::Move(whitePly,blackPly, moveCount));
			
			++moveCount;
			whitePly = pgn::Ply();
			blackPly = pgn::Ply();
			pendingMove = false;
		}
		
		_p.doMove(res.PV.getMove(0));
		
		_c.switchTurn();
	}
	// white move still to be written
	if(_c.isBlackTurn()) {
		pgnGame.moves().push_back(pgn::Move(whitePly,blackPly, moveCount));
	}
	
	_addGameResult(pgnGame,_getGameResult());
	
	return pgnGame;
}

bool SelfPlay::_isGameFinished() {
	// checkmate
	if (_p.isCheckMate()) {
		return true;
	}
	
	// patta ripetizione
	// num mosse
	if (_p.isDraw(true)) {
		return true;
	}
	
	// stallo
	if (_p.isStaleMate()) {
		return true;
	}
	
	// early stop
	// near 0 for x moves after ply y
	// abs(v) > x  for y moves
	
	return false;
}

std::string SelfPlay::_getGameResult() {
	// checkmate
	if (_p.isCheckMate()) {
		if(_p.isBlackTurn()) {
			return "1-0";
		} else {
			return "0-1";
		}
	}
	
	// patta ripetizione
	// num mosse
	if (_p.isDraw(true)) {
		return "1/2-1/2";
	}
	
	// stallo
	if (_p.isStaleMate()) {
		return "1/2-1/2";
	}
	
	// early stop
	// near 0 for x moves after ply y
	// abs(v) > x  for y moves
	
	return "*";
}

void SelfPlay::_addGameTags(pgn::Game& g, int round) {
	g.tags().insert(pgn::Tag("Event","autplay"));
	g.tags().insert(pgn::Tag("Site","Florence"));
	g.tags().insert(pgn::Tag("Date","2019.11.01"));
	g.tags().insert(pgn::Tag("Round",std::to_string(round)));
	g.tags().insert(pgn::Tag("White",_white.getName()));
	g.tags().insert(pgn::Tag("Black",_black.getName()));
	g.tags().insert(pgn::Tag("Result","*"));
	
}
void SelfPlay::_addGameResult(pgn::Game& g, const std::string & s) {
	g.tags().insert(pgn::Tag("Result",s));
	g.result() = s;
}