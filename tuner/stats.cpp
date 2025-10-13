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
		int16_t d;
		char c[2];
	}bb;

	for(unsigned int i = 0; i < featuresCount; ++i) {
		f.read(bb.c,2);
		unsigned int feat = bb.d;
		d.f.add(feat);
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

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load("nnue.par")) {
		std::cout<<"error loading nnue.par"<<std::endl;

	}

	std::string line;
	std::ifstream myfile ("fen.data");

	if (myfile.is_open()) {

		Data d = getPosition(myfile);

		while ( d.f.size() != 0 ) {

			pos.setupFromFeatureList(d.f);
			//position counter;
			posCounter++;
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
		for(auto& [key, value]: endgames) {
			std::cout<<pos.getStandardEgFenFromKey(key)<<" : "<<value<<std::endl;
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

	worker();

	return 0;
}
