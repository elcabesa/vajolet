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
#include <map>
#include <cmath>

//#include "book.h"
//#include "epdSaver.h"
//#include "fenSaver.h"
//#include "player.h"
#include "position.h"
//#include "tournament.h"
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

unsigned int v01 = 0;
unsigned int v02 = 0;
unsigned int v03 = 0;
unsigned int v04 = 0;
unsigned int v05 = 0;
unsigned int v06 = 0;
unsigned int v07 = 0;
unsigned int v08 = 0;
unsigned int v09 = 0;
unsigned int v10 = 0;

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
					for(auto& idx: f) {
						stats[idx]++;
						_stream <<idx<<" ";
					}
					/*for(unsigned int i = 0; i< 64*12; ++i) {
						if(std::find(f.begin(), f.end(),i)!= f.end()) {
							stats[i]++;
							_stream <<1<<" ";
						}else {
							_stream <<0<<" ";
						}
					}*/
					double dval = std::stoi(val)/50000.0;
					//dval = std::max(-20.0, dval);
					//dval = std::min(dval, 20.0);
					dval = 1.0/(1 + std::exp(-1.0 * dval)) ;
					_stream<<";"<<dval<<std::endl;
					double abs = std::abs(dval);
					max = std::max(abs,max);
					if(abs <= 0.1) {++v01;}
					else if(abs <= 0.1) {++v01;}
					else if(abs <= 0.2) {++v02;}
					else if(abs <= 0.3) {++v03;}
					else if(abs <= 0.4) {++v04;}
					else if(abs <= 0.5) {++v05;}
					else if(abs <= 0.6) {++v06;}
					else if(abs <= 0.7) {++v07;}
					else if(abs <= 0.8) {++v08;}
					else if(abs <= 0.9) {++v09;}
					else {++v10;}

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
	std::cout<<"<=0,1 "<<v01<<std::endl;
	std::cout<<"<=0,2 "<<v02<<std::endl;
	std::cout<<"<=0,3 "<<v03<<std::endl;
	std::cout<<"<=0,4 "<<v04<<std::endl;
	std::cout<<"<=0,5 "<<v05<<std::endl;
	std::cout<<"<=0,6 "<<v06<<std::endl;
	std::cout<<"<=0,7 "<<v07<<std::endl;
	std::cout<<"<=0,8 "<<v08<<std::endl;
	std::cout<<"<=0,9 "<<v09<<std::endl;
	std::cout<<"<=1,0 "<<v10<<std::endl;


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
