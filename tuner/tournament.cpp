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


const std::string Tournament::_pgnName = "tournament.pgn";

Tournament::Tournament(): _p1("dev"), _p2("ref") {
	_p1.getSearchParameters().razorMargin = 20000;
	_p1.getSearchParameters().razorMarginDepth = 0;
	_p1.getSearchParameters().razorMarginCut = 0;
}

void Tournament::play() {
	std::cout<<"start tournament ("<< TunerParameters::gameNumber << " games)" << std::endl;
	_createNewTournamentPgn();
	Stats stats;
	
	for(int i = 0; i < TunerParameters::gameNumber; ++i) {
		Player* whitePlayer;
		Player* blackPlayer;
		
		unsigned int round = i + 1;
		
		if( round % 2) {
			whitePlayer = &_p1;
			blackPlayer = &_p2;
		} else {
			whitePlayer = &_p2;
			blackPlayer = &_p1;
		}
		std::cout<< "starting game " <<round  <<" of "<< TunerParameters::gameNumber << "(" << round * 100.0 / TunerParameters::gameNumber << "%) ";
		auto g = SelfPlay(*whitePlayer, *blackPlayer).playGame(round);
		
		stats.insert(g);
		std::cout<< stats.print()<<" ";
		if(g.result().isWhiteWin()) {
			whitePlayer->insertResult(1);
			blackPlayer->insertResult(-1);
		} else if(g.result().isBlackWin()) {
			whitePlayer->insertResult(-1);
			blackPlayer->insertResult(1);
		} else if(g.result().isDrawn()) {
			whitePlayer->insertResult(0);
			blackPlayer->insertResult(0);
		} else {
			whitePlayer->insertResult(-2);
			blackPlayer->insertResult(-2);
		}
		std::cout<< _p1.print()<<" ";
		std::cout<< _p2.print()<<" ";
		std::cout<<std::endl;
		
		_saveGamePgn(g);
	}
	std::cout<< std::endl << "finished tournament!" <<std::endl;
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
