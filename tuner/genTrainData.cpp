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
#include <fstream>
#include <string>

#include "book.h"
#include "epdSaver.h"
#include "fenSaver.h"
#include "player.h"
#include "position.h"
#include "tournament.h"
#include "nnue.h"

#include "libchess.h"
#include "vajo_io.h"

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
	std::cout <<"Vajolet train data generator"<< std::endl;
}


void worker2() {
	double max = 0;
	std::map<int, int> stats;
	std::ofstream _stream;
	std::ofstream _stream2;
	unsigned long long int count = 0;
	_stream.open("fen.data");
	_stream2.open("header.data");
//	FenSaver fs(1, th);
	Position pos(Position::nnueConfig::on);
    std::string line;
	for( unsigned int th = 1; th <=4; ++th) {
		std::ifstream myfile ("fen" + std::to_string(th) + ".epd");
		if (myfile.is_open())
		{
			//unsigned int x = 0;
			while ( getline (myfile,line) )
			{
				++count;
				//std::cout <<"THREAD "<<th<< " got line "<< line <<std::endl;
				auto sep = line.find_first_of(';');
				if(sep != std::string::npos) {
					auto fen = line.substr(0, sep);
					auto val = line.substr(sep+1);
					pos.setupFromFen(fen);
					auto f = pos.nnue()->features();
					for(unsigned int i = 0; i< 64*12; ++i) {
						if(std::find(f.begin(), f.end(),i)!= f.end()) {
							stats[i]++;
							_stream <<1<<" ";
						}else {
							_stream <<0<<" ";
						}
					}
					double dval = std::stoi(val)/10000.0;
					dval = std::max(-20.0, dval);
					dval = std::min(dval, 20.0);
					_stream<<std::endl<<dval<<std::endl;
					max = std::max(std::abs(dval),max);

				}
	//            fs.save(pos.setupFromFen(line));
			}
			//std::cout <<"THREAD "<<th<<  "FINISHED "<< std::endl;
			myfile.close();
		}
		else std::cout << "Unable to open file"<<std::endl;
	}

	_stream2 <<count<<" 768 1"<<std::endl;
	_stream.close();
	_stream2.close();

	for(unsigned int i = 0 ;i < 64 * 12; ++i) {
		auto t = stats[i];
		std::cout<<i <<" "<<t<<std::endl;
	}

	std::cout<<"MAX "<<max<<std::endl;

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
