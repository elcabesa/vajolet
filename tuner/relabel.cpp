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

#include <csignal>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <map>

#include "position.h"
#include "nnue.h"
#include "featureList.h"
#include "player.h"

#include "libchess.h"

void signalHandler(int signum)
{
	exit(signum);
}

/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	std::cout <<"Vajolet verify"<< std::endl;
}

struct Data {
	std::string fen;
	int v;
};

//todo create saver

Data getPosition(std::ifstream& f) {
	//TODO insert mutex
	Data d ={"",0};
	std::string line;
	std::getline(f, line);

	if(f.eof()) {
		return d;
	}

	int n = 0;

	std::stringstream test(line);
	std::string segment;
	while(std::getline(test, segment, '|'))
	{
		if( n == 0) {
			d.fen = segment;
		} else {
			d.v = std::stoi(segment);
			break;

		}
		++n;
	}

	return d;
}

bool worker() {
	SearchLimits sl;

	Player p("white");
	std::string path = "internal";

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load(path)) {
		std::cout<<"error loading "<<path<<std::endl;
		return false;
	}

	std::string line;
	std::ifstream myfile ("fen.data");

	unsigned long poscount = 0;
	float maxError = 0;

	if (myfile.is_open()) {

		Data d = getPosition(myfile);

		while ( d.fen != "") {
			++poscount;
			if(poscount%1000 == 0) {
				std::cout<<poscount/1e6<<"M maxError "<<maxError<<std::endl;
			}

			pos.setupFromFen(d.fen);
			auto res = p.doSearch(pos, sl);
			Score score = res.Res;
			if(pos.isBlackTurn()) {
				score = -score;
			}


			auto dval = d.v*100;


			if(std::abs(score) < SCORE_KNOWN_WIN) {
				float error = std::abs(score - dval);
				// TODO save value
				if(error > maxError) {
					maxError = error;
					std::cout<<"new "<<score<<" old "<<dval<<std::endl;
				}
			} else {
				std::cout<<"found win "<<score<<std::endl;
				// TODO save as max value
			}

			d = getPosition(myfile);
		}
		myfile.close();
	}
	else std::cout << "Unable to open file"<<std::endl;

	return true;

}

int main() {
	
	signal(SIGINT, signalHandler); 
#ifdef SIGBREAK	
	signal(SIGBREAK, signalHandler); 
#endif
#ifdef SIGHUP		
	signal(SIGHUP, signalHandler);  
#endif

	printStartInfo();
	//----------------------------------
	//	init global data
	//----------------------------------
	libChessInit();

	//TODO do search in paralles (launch n thread)
	worker();

	return 0;
}
