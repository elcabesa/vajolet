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
#include <iostream>
#include <fstream>
#include <string>

#include "book.h"
#include "PGNGame.h"
#include "selfplay.h"
#include "tournament.h"
#include "tunerPars.h"

class Stats {
public:
	Stats() {};
	void insert(const pgn::Game& g) {
		const auto& res = g.result();
		_whiteWin += res.isWhiteWin();
		_blackWin += res.isBlackWin();
		_draw += res.isDrawn();
		_unknown += res.isUnknown();
	}
	
	std::string print() {
		std::string s;
		s += std::to_string(_whiteWin);
		s += "/";
		s += std::to_string(_blackWin);
		s += "/";
		s += std::to_string(_draw);
		s += " ";
		s += std::to_string(_unknown);
		return s;
	}
private:
	int _whiteWin = 0;
	int _blackWin = 0;
	int _draw = 0;
	int _unknown = 0;
};

Tournament::Tournament(const std::string& pgnName, const std::string& debugName, Player& p1, Player& p2, Book& b, FenSaver * const fs,  bool verbose): _pgnName(pgnName), _debugName(debugName), _p1(p1), _p2(p2), _book(b), _fs(fs),_verbose(verbose) {
}

TournamentResult Tournament::play() {
	std::ofstream myfile;
	myfile.open (_debugName);
	myfile<<"start tournament ("<< TunerParameters::gameNumber << " games)" << std::endl;
	_createNewTournamentPgn();
	Stats stats;
	Player* whitePlayer = &_p1;
	Player* blackPlayer = &_p2;
	
	for(int i = 0; i < TunerParameters::gameNumber; ++i) {
		unsigned int round = i + 1;
		
		if(round % 2) {
			whitePlayer = &_p1;
			blackPlayer = &_p2;
		} else {
			whitePlayer = &_p2;
			blackPlayer = &_p1;
		}
		myfile<< "starting game " <<round  <<" of "<< TunerParameters::gameNumber << "(" << round * 100.0 / TunerParameters::gameNumber << "%) ";
		auto g = SelfPlay(*whitePlayer, *blackPlayer, _book, _fs).playGame(round);
		
		stats.insert(g);
		_updateResults(g, *whitePlayer, *blackPlayer);
		
		myfile<< _p1.print()<<" ";
		myfile<< stats.print()<<" ";
		myfile<<std::endl;
		if (_verbose) { std::cout << "game " << round << std::endl;}
		
		_saveGamePgn(g);
	}
	myfile<< std::endl << "finished tournament!" <<std::endl;
	double res = _p1.pointRatio();
	if(res > 0.51) {
		return TournamentResult::p1Won;
	} else if (res < 0.49) {
		return TournamentResult::p2Won;
	} else {
		return TournamentResult::draw;
	}
	myfile.close();
}

void Tournament::_saveGamePgn(const pgn::Game& g) {
	std::ofstream myfile;
	myfile.open(_pgnName, std::fstream::app);
	myfile<<g<<std::endl;
	myfile.close();
}

void Tournament::_createNewTournamentPgn() {
	std::ofstream myfile;
	myfile.open(_pgnName, std::fstream::out);
	myfile.close();
}

void Tournament::_updateResults(const pgn::Game& g, Player& w, Player& b) {
	if(g.result().isWhiteWin()) {
		w.insertResult(1);
		b.insertResult(-1);
	} else if(g.result().isBlackWin()) {
		w.insertResult(-1);
		b.insertResult(1);
	} else if(g.result().isDrawn()) {
		w.insertResult(0);
		b.insertResult(0);
	} else {
		w.insertResult(-2);
		b.insertResult(-2);
	}
}
