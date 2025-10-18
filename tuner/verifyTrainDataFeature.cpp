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
	FeatureList f;
	float v;
};

Data getPosition(std::ifstream& f) {
	Data d;
	char buffer[10];
	unsigned int featuresCount;

	f.read(buffer,1);
	featuresCount = buffer[0];

	if(f.eof()) {
		return d;
	}


	union _bb{
		char c[64];
		int16_t d[32];

	}bb;
	f.read(bb.c, 2 * featuresCount);

	for(unsigned int i = 0; i < featuresCount; ++i) {
		d.f.add(bb.d[i]);
	}

	union _bbf{
		float v;
		char c[4];
	}bbf;

	f.read(bbf.c,4);
	d.v = bbf.v;
	return d;
}

void worker() {
	bool loaded = false;
	std::map<int, unsigned long> count;
	std::map<int, double> mse;
	std::map<int, double> mseSigmoid;

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load("nnue.par")) {
		std::cout<<"error loading nnue.par"<<std::endl;
	} else {
		loaded = true;
	}

	std::string line;
	std::ifstream myfile ("fen.data");

	unsigned long poscount = 0;
	if (myfile.is_open()) {

		Data d = getPosition(myfile);

		while ( d.f.size() != 0 ) {
			++poscount;
			//std::cout<<poscount<<std::endl;
//#define DEBUG_PARSER
#ifdef DEBUG_PARSER
			for(unsigned int i = 0; i < d.f.size(); ++i) {
				std::cout <<d.f.get(i)<<" ";
			}
			std::cout<<";"<<d.v<<std::endl;
#endif

			double dScore;
			pos.setupFromFeatureList(d.f);
			if(loaded) {
				dScore = pos.nnue()->eval()/10000.0;
			} else {
				dScore = pos.eval<false>()/10000.0;
			}

			if(pos.isBlackTurn()) {
				dScore = -dScore;
			}

			auto dval = d.v/10000.0;
			int diff = std::abs(dScore-dval)*10;
			/*if(diff >=100) {
				std::cout<<pos.getFen()<< " "<<dScore<<";"<<dval<<std::endl;
			}*/



			count[diff]++;

			mse[diff] += std::pow(dScore-dval,2.0);

			double dScoreSigmoid = dScore /5.0;
			dScoreSigmoid = 1.0/(1 + std::exp(-1.0 * dScoreSigmoid));

			double dvalSigmoid = dval /5.0;
			dvalSigmoid = 1.0/(1 + std::exp(-1.0 * dvalSigmoid));

			mseSigmoid[diff] += std::pow(dScoreSigmoid-dvalSigmoid,2.0);

			d = getPosition(myfile);
		}
		myfile.close();
	}
	else std::cout << "Unable to open file"<<std::endl;

	std::cout<<"count:"<<poscount<<std::endl;

	double totMse = 0;
	double totMseSigmoid = 0;
	unsigned long totCount = 0;
	unsigned long prevCount = 0;
	for(unsigned int i = 0; i<400; ++i) {
		totMse += mse[i];
		totMseSigmoid += mseSigmoid[i];
		totCount+= count[i];
		if((i<40) | (i ==399) ) {
			std::cout<<i*0.1<<" ";
			std::cout<<"Positions: "<<totCount - prevCount <<" ";
			std::cout<<"MSE: "<<totMse/totCount<<" ";
			std::cout<<"MSESigmoid: "<<totMseSigmoid/totCount<<" ";
			std::cout<<"totcount: "<<totCount<<std::endl;
			prevCount = totCount;
		}
	}
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

	worker();

	return 0;
}
