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
#include "clock.h"
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

SelfPlay::SelfPlay() : _p(Position::pawnHash::off), _c(TunerParameters::gameTime, TunerParameters::gameTimeIncrement) {
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
	
	// init vajolet search & time management
	my_thread &thr = my_thread::getInstance();
	thr.setMute(true);
	
	
	
	pgn::Game pgnGame;
	_addGameTags(pgnGame, round);
	
	int i = 0;
	int count = i/2;
	_c.start();
	
	pgn::Ply whitePly;
	pgn::Ply blackPly;
	bool pendingMove = false;
	
	while (!_isGameFinished()) {
		// set time limit
		_sl.setWTime(_c.getWhiteTime());
		_sl.setBTime(_c.getBlackTime());
		
		count = i/2;
		if( i%2 == 0) {
			std::cout<< count + 1<<"."<<std::endl;
		}
		
		// do the search
		auto begin = std::chrono::high_resolution_clock::now();
		thr.startThinking(_p, _sl);
		thr.finished().wait(lock);
		auto end = std::chrono::high_resolution_clock::now();
		
		auto dur = end - begin;
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		
		auto res = thr.getResult();
		
		
		
		_c.switchTurn();
		std::cout<<UciManager::displayMove(_p, res.PV.getMove(0))<<"("<<ms<<") depth: " <<res.depth<<" score: "<<((i%2 == 0) ? res.Res: -res.Res) <<" white_time: "<<_c.getWhiteTime()<<" black_time: "<<_c.getBlackTime()<<std::endl;
		
		
		if( i%2 == 0) {
			whitePly = pgn::Ply(UciManager::displayMove(_p, res.PV.getMove(0)));
			pendingMove =true;
		}
		else {
			blackPly = pgn::Ply(UciManager::displayMove(_p, res.PV.getMove(0)));
			pgnGame.moves().push_back(pgn::Move(whitePly,blackPly, count + 1));

			whitePly = pgn::Ply();
			blackPly = pgn::Ply();
			pendingMove = false;
		}
		
		_p.doMove(res.PV.getMove(0));
		
		++i;
	}
	
	if(pendingMove) {
		pgnGame.moves().push_back(pgn::Move(whitePly,blackPly, count + 1));
	}
	 // todo calulate the right result
	_addGameResult(pgnGame,_getGameResult());
	
	return pgnGame;
}

bool SelfPlay::_isGameFinished() {
	// checkmate
	if (_p.isCheckMate()) {
		std::cout<<std::endl<<"CHECKMATE"<<std::endl;
		return true;
	}
	
	// patta ripetizione
	// num mosse
	if (_p.isDraw(true)) {
		std::cout<<std::endl<<"DRAW"<<std::endl;
		return true;
	}
	
	// stallo
	if (_p.isStaleMate()) {
		std::cout<<std::endl<<"STALEMATE"<<std::endl;
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
	g.tags().insert(pgn::Tag("White","Vajolet"));
	g.tags().insert(pgn::Tag("Black","Vajolet"));
	g.tags().insert(pgn::Tag("Result","1-0"));
	
}
void SelfPlay::_addGameResult(pgn::Game& g, const std::string & s) {
	g.tags().insert(pgn::Tag("Result",s));
}