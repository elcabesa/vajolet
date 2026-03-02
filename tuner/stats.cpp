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
#include <iomanip>

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
	std::cout <<"Vajolet stat"<< std::endl;
}

// positions counter
unsigned long long posCounter = 0;
//king squares
unsigned long long wksquare[tSquare::squareNumber] = {0};
unsigned long long bksquare[tSquare::squareNumber] = {0};
//piece count stats
unsigned long long pieceCount[tSquare::squareNumber] = {0};
//imbalancies
std::map<int,unsigned long long> imbalancies;
std::map<tKey, unsigned long long> endgames;

struct Data {
	FeatureList f;
	std::string fen;
	float v;
};

Data getPosition(std::ifstream& f) {

	Data d;

	if(f.eof()) {
		return d;
	}

	std::string line;
	std::getline(f, line);
	//std::cout<<line<<std::endl;

	std::vector<std::string> output;

	std::string::size_type prev_pos = 0, pos = 0;

	while((pos = line.find('|', pos)) != std::string::npos)
	{
		std::string substring( line.substr(prev_pos, pos-prev_pos) );

		output.push_back(substring);

		prev_pos = ++pos;
	}

	output.push_back(line.substr(prev_pos, pos-prev_pos)); // Last word
	if(output.size() <3) {
		return d;
	}

	//std::cout<<output[0]<<std::endl;
	//std::cout<<output[1]<<std::endl;
	d.fen = output[0];
	d.v = std::stoi(output[1])/100.0;


	return d;
}

bool worker(unsigned int i) {

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load("internal")) {
		std::cout<<"error loading nnue.par"<<std::endl;

	}

	std::string line;

	std::stringstream ss;
	ss << std::setw(3) << std::setfill('0') << i;
	std::string s = ss.str();
	std::string fileName = "fen-";
	fileName += s;
	fileName += ".data";
	std::cout<<fileName<<std::endl;


	std::ifstream myfile (fileName);

	pos.setupFromFen("kp6/8/8/8/8/8/8/7K w - -");
	auto kpk = pos.getMaterialKey().getKey();
	//std::cout<<kpk<<std::endl;
	//return;

	if (myfile.is_open()) {

		Data d = getPosition(myfile);

		while ( d.fen.size() != 0 ) {
			pos.setupFromFen(d.fen);

			//position counter;
			posCounter++;

			if(posCounter%1000000 == 0) {
				std::cout<<posCounter/1e6<<"M"<<std::endl;
			}

			//ksquares
			++wksquare[pos.getSquareOfThePiece(whiteKing)];
			++bksquare[pos.getSquareOfThePiece(blackKing)];
			//piececount
			++pieceCount[pos.getPieceCount(occupiedSquares)];
			//imbalancies
			signed int imb = pos.getPieceCount(whiteQueens) * 9
			+pos.getPieceCount(whiteRooks) * 5
			+pos.getPieceCount(whiteBishops) * 3
			+pos.getPieceCount(whiteKnights) * 3
			+pos.getPieceCount(whitePawns) * 1
			-pos.getPieceCount(blackQueens) * 9
			-pos.getPieceCount(blackRooks) * 5
			-pos.getPieceCount(blackBishops) * 3
			-pos.getPieceCount(blackKnights) * 3
			-pos.getPieceCount(blackPawns) * 1;
			imbalancies[imb]++;
			//endgames
			if( pos.isMaterialDataAKnownEndgame() ) {
				endgames[pos.getMaterialKey().getKey()]++;
				/*if(pos.getMaterialKey().getKey() == kpk) {
					//if(d.v )
					std::cout<<"fen "<<d.fen<<" value "<<d.v<<std::endl;
				}*/
			}
			d = getPosition(myfile);
		}
		myfile.close();

		std::cout<<"position counter: "<<posCounter<<std::endl;
		std::cout<<"wk squares: "<<std::endl;
		for(tRank r = RANK1; r < rankNumber; ++r) {
			for(tFile f = FILEA; f < fileNumber; ++f) {
				std::cout<<wksquare[getSquare(f,r)]<<"  ";
			}
			std::cout<<std::endl;
		}
		std::cout<<"bk squares: "<<std::endl;
		for(tRank r = RANK1; r < rankNumber; ++r) {
			for(tFile f = FILEA; f < fileNumber; ++f) {
				std::cout<<bksquare[getSquare(f,r)]<<"  ";
			}
			std::cout<<std::endl;
		}

		std::cout<<"piececount"<<std::endl;
		for(tSquare sq = square0; sq <squareNumber; ++sq) {
			if(pieceCount[sq]) std::cout<<sq<<":"<<pieceCount[sq]<<std::endl;
		}
		std::cout<<"imbalancies"<<std::endl;
		for(auto& [key, value]: imbalancies) {
			std::cout<<key<<" "<<value<<std::endl;
		}


		std::cout<<"endgames"<<std::endl;
		std::multimap<unsigned long long, tKey> sortedEndgames;
		for(auto& [key, value]: endgames) {
			sortedEndgames.insert({value, key});
		}
		for(auto& [key, value]: sortedEndgames) {
			std::cout<<pos.getStandardEgFenFromKey(value)<<" : "<<key<<std::endl;
		}
		std::cout<<"return true"<<std::endl;
		return true;
	}
	else std::cout << "Unable to open file"<<std::endl;
	return false;
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

	for(unsigned int i = 1; worker(i);++i) {}

	return 0;
}
