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

void worker2() {

	const double stateScale = 1.0;

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load("nnue.par")) {
		std::cout<<"error loading nnue.par"<<std::endl;
		//exit(-1);
	}

    std::string line;
	std::ifstream myfile ("fenver.epd");

	std::map<int, unsigned long> stat;

	unsigned long count = 0;
	double mse = 0;
	double mseSigmoid = 0;

	if (myfile.is_open()) {

		while ( getline (myfile,line) ) {

			++count;

			auto sep = line.find_first_of(';');
			if(sep != std::string::npos) {
				auto val = line.substr(sep+1);

				/*auto features = line.substr(0, sep);
				std::istringstream stream(features);
				// Temporary string to store each token
				std::string token;

				FeatureList fl;

				// Read tokens from the string stream separated by the
				// delimiter
				while (getline(stream, token, ' ')) {
					// Add the token to the array
					fl.add(std::stoi(token));
				}

				auto s = nnue->eval(fl);
				double dScore = s /50000.0;
				dScore = 1.0/(1 + std::exp(-1.0 * dScore));

				double dval = std::stof(val);

				mse += std::pow(dScore-dval,2.0);

				stat[std::abs(dScore-dval)/stateScale] ++;

				if(std::abs(dScore-dval) > 0.7) {
					std::cout<<dScore<<" "<<dval<<std::endl;
				}*/

				auto fen = line.substr(0, sep);
				pos.setupFromFen(fen);

				auto dScore = pos.eval<false>()/10000.0;
				if(pos.isBlackTurn()) {
					dScore = -dScore;
				}

				auto dval = std::stoi(val)/10000.0;

				//std::cout<<dScore<<" "<<dval<<std::endl;

				mse += std::pow(dScore-dval,2.0);

				stat[(dScore-dval)/stateScale] ++;

				if(std::abs(dScore-dval) > 15) {
					std::cout<<fen <<" "<< dScore<<" "<<dval<<std::endl;
				}

				double dScoreSigmoid = dScore /5.0;
				dScoreSigmoid = 1.0/(1 + std::exp(-1.0 * dScoreSigmoid));

				double dvalSigmoid = dval /5.0;
				dvalSigmoid = 1.0/(1 + std::exp(-1.0 * dvalSigmoid));

				mseSigmoid += std::pow(dScoreSigmoid-dvalSigmoid,2.0);




			}

		}
		myfile.close();
		std::cout<<"Positions: "<<count<<std::endl;
		std::cout<<"MSE: "<<mse/count<<std::endl;
		std::cout<<"MSESigmoid: "<<mseSigmoid/count<<std::endl;
		for(const auto& [key, value] :stat) {
			std::cout<<key*stateScale<<" "<<value<<std::endl;
		}
	}
	else std::cout << "Unable to open file"<<std::endl;
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

	worker2();

	return 0;
}
