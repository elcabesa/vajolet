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

#include <string>

#include "book.h"
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
#include "tunerPars.h"
#include "uciOutput.h"

SelfPlay::SelfPlay(Player& white, Player& black, Book& b) : _p(Position::pawnHash::off), _c(TunerParameters::gameTime, TunerParameters::gameTimeIncrement), _white(white), _black(black), _book(b) {
	_p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	_sl.setWTime(_c.getWhiteTime());
	_sl.setBTime(_c.getBlackTime());
	
	_sl.setWInc(TunerParameters::gameTimeIncrement * 1000);
	_sl.setBInc(TunerParameters::gameTimeIncrement * 1000);
}

pgn::Game SelfPlay::playGame(unsigned int round) {
	
	pgn::Game pgnGame;
	_addGameTags(pgnGame, round);
	
	int moveCount = 1;
	_c.start();
	
	pgn::Ply whitePly;
	pgn::Ply blackPly;
	bool pendingMove = false;
	
	// read book moves
	auto bookMoves = _book.getLine();
	auto it = bookMoves.begin();
	
	while (!_isGameFinished()) {
		// set time limit
		_sl.setWTime(_c.getWhiteTime());
		_sl.setBTime(_c.getBlackTime());
		
		Move bestMove;
		if(it != bookMoves.end())
		{
			// make book moves
			bestMove = *it;
			++it;
		}
		else 
		{	// out of book search
			auto res = _c.isWhiteTurn()? _white.doSearch(_p, _sl): _black.doSearch(_p, _sl);
			bestMove = res.PV.getMove(0);
		}
		
		if(_c.isWhiteTurn()) {
			whitePly = pgn::Ply(UciOutput::displayMove(_p, bestMove));
			pendingMove =true;
		}
		else
		{
			blackPly = pgn::Ply(UciOutput::displayMove(_p, bestMove));
			pgnGame.moves().push_back(pgn::Move(whitePly,blackPly, moveCount));
			
			++moveCount;
			whitePly = pgn::Ply();
			blackPly = pgn::Ply();
			pendingMove = false;
		}
		
		_p.doMove(bestMove);
		
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