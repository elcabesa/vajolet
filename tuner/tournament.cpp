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


Tournament::Tournament(bool& stop, Player& p1, Player& p2, Book& b, BinSaver * const fs,  bool verbose): /*_pgnName(pgnName), _debugName(debugName),*/ _p1(p1), _p2(p2), _book(b), _fs(fs),_verbose(verbose), _stop(stop) {
}

void Tournament::play() {

	for(unsigned int i = 0; i < ALREADYSEEN_SIZE; ++i) {
		alreadySeen[i] = 0;
	}

	unsigned int logDecimation = 0;
	Player* whitePlayer = &_p1;
	Player* blackPlayer = &_p2;
	
	for(int i = 0; (i < TunerParameters::gameNumber || TunerParameters::gameNumber == -1) && !_stop; ++i) {
		unsigned int round = i + 1;
		
		if(round % 2) {
			whitePlayer = &_p1;
			blackPlayer = &_p2;
		} else {
			whitePlayer = &_p2;
			blackPlayer = &_p1;
		}
		SelfPlay(*whitePlayer, *blackPlayer, _book, alreadySeen, _fs).playGame();
		
		if (_verbose) { 
			if( ++logDecimation >= 100) {
				logDecimation = 0;
				std::cout << "game " << round << std::endl;}
			}
	}
	_finished = true;
}
