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

bool worker(int i) {
	std::string path = std::string("file")+ std::to_string(i) + std::string(".weight");
	bool loaded = false;
	std::map<int, unsigned long> count;
	std::map<int, double> mse;
	std::map<int, double> mseSigmoid;

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load(path)) {
		std::cout<<"error loading "<<path<<std::endl;
		return false;
	} else {
		loaded = true;
	}

	std::string line;
	std::ifstream myfile ("fen.data");

	unsigned long poscount = 0;
	unsigned long errcount = 0;
	double mseError = 0;
	double mseErrorSigmoid = 0;

	if (myfile.is_open()) {

		Data d = getPosition(myfile);

		while ( d.f.size() != 0 ) {
			++poscount;
			if(poscount%1000000 == 0) {
				std::cout<<poscount/1e6<<"M"<<std::endl;
			}
//#define DEBUG_PARSER
#ifdef DEBUG_PARSER
			for(unsigned int i = 0; i < d.f.size(); ++i) {
				std::cout <<d.f.get(i)<<" ";
			}
			std::cout<<";"<<d.v<<std::endl;
#endif

			double dScore;
#define FASTNN
#ifdef FASTNN

			if(loaded) {
				dScore = pos.nnue()->eval(d.f)/10000.0;
			} else {
				pos.setupFromFeatureList(d.f);
				dScore = pos.eval<false>()/10000.0;
			}
#else

			pos.setupFromFeatureList(d.f);
			if(loaded) {
				dScore = pos.nnue()->eval()/10000.0;
			} else {
				dScore = pos.eval<false>()/10000.0;
			}
#endif

			auto dval = d.v/10000.0;
			auto ddiff = std::abs(dScore-dval);
/*			if(ddiff >=10) {
				pos.setupFromFeatureList(d.f);
				std::cout<<pos.getFen()<< " "<<dScore<<";"<<dval<<std::endl;
			}*/
			int diff = ddiff * 10;



			count[diff]++;

			double se  = std::pow(dScore-dval,2.0);
			mse[diff] += se;

			double dScoreSigmoid = dScore /5.0;
			dScoreSigmoid = 1.0/(1 + std::exp(-1.0 * dScoreSigmoid));

			double dvalSigmoid = dval /5.0;
			dvalSigmoid = 1.0/(1 + std::exp(-1.0 * dvalSigmoid));
			double err = std::pow(dScoreSigmoid-dvalSigmoid,2.0);
			mseSigmoid[diff] += err;

			if(
				ddiff >=4
				&& err > 0.04
				&& dScore * dval <0
			) {
				errcount++;
				mseError+= se;
				mseErrorSigmoid+= err;
				/*pos.setupFromFeatureList(d.f);
				std::cout<<pos.getFen()<< " "<<dScore<<";"<<dval<<" "<<err<<std::endl;*/
			}

			d = getPosition(myfile);
		}
		myfile.close();
	}
	else std::cout << "Unable to open file"<<std::endl;

	std::cout<<"count:"<<poscount<<" err:"<<errcount<<std::endl;

	double totMse = 0;
	double totMseSigmoid = 0;
	unsigned long totCount = 0;
	unsigned long prevCount = 0;
	for(unsigned int i = 0; i<400; ++i) {
		totMse += mse[i];
		totMseSigmoid += mseSigmoid[i];
		totCount+= count[i];
		if((i<40) | (i ==399) )
		{
			std::cout<<i*0.1<<" ";
			std::cout<<"Positions: "<<totCount - prevCount <<" ";
			std::cout<<"MSE: "<<totMse/totCount<<" ";
			std::cout<<"MSESigmoid: "<<totMseSigmoid/totCount<<" ";
			std::cout<<"totcount: "<<totCount<<std::endl;
			prevCount = totCount;
		}
	}
	for(unsigned int i = 0; i<400; ++i) {
		if((i<40) | (i ==399) )
		{
			std::cout<<i*0.1<<" ";
			std::cout<<"Positions: "<<totCount - prevCount <<" ";
			std::cout<<"MSE: "<<mse[i]<<" ";
			std::cout<<"MSESigmoid: "<<mseSigmoid[i]<<" ";
			std::cout<<"totcount: "<<totCount<<std::endl;
			prevCount = totCount;
		}
	}
	std::cout<<"filtered error "<<mseError/totCount <<" "<<mseErrorSigmoid/totCount<<std::endl;
	std::cerr<<i<<";"<<totMse/totCount<<";"<<totMseSigmoid/totCount<<";"<<errcount<<std::endl;

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

	for(unsigned int i = 1; worker(i); ++i) {
	}

	return 0;
}
