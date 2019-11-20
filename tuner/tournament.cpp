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
		whiteWin += res.isWhiteWin();
		blackWin += res.isBlackWin();
		draw += res.isDrawn();
		unknown += res.isUnknown();
	}
	
	std::string print() {
		std::string s;
		s += std::to_string(whiteWin);
		s += "/";
		s += std::to_string(blackWin);
		s += "/";
		s += std::to_string(draw);
		s += " ";
		s += std::to_string(unknown);
		return s;
	}
private:
	int whiteWin = 0;
	int blackWin = 0;
	int draw = 0;
	int unknown = 0;
};


const std::string Tournament::_pgnName = "tournament.pgn";

Tournament::Tournament() {}

void Tournament::play() {
	std::cout<<"start tournament ("<< TunerParameters::gameNumber << " games)" << std::endl;
	_createNewTournamentPgn();
	Stats stats;
	
	for(int i = 0; i < TunerParameters::gameNumber; ++i) {
		unsigned int round = i + 1;
		std::cout<< "\rstarting game " <<round  <<" of "<< TunerParameters::gameNumber << "(" << round * 100.0 / TunerParameters::gameNumber << "%) ";
		auto g = SelfPlay().playGame(round);
		
		stats.insert(g);
		std::cout<< stats.print();
		
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
