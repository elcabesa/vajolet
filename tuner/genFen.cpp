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
#include <thread>
#include <chrono>
#include <future>

#include "book.h"
#include "txtSaver.h"
#include "player.h"
#include "position.h"
#include "tournament.h"
#include "nnue.h"

#include "libchess.h"
#include "vajo_io.h"

bool stop = false;

void signalHandler(int /*signum*/)
{
	std::cout<<"STOP"<<std::endl;
	stop = true;
}

/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	std::cout <<"Vajolet fen generator"<< std::endl;
}

std::ofstream stream;

void worker() {
	TxtSaver fs(1, stream);
	Book book("book.pgn");
	Player p1("p1");
	Player p2("p2");
	Tournament t(stop, p1, p2, book, &fs, true);
	t.play();

}

int main() {
	using namespace std::chrono_literals;
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

	Position pos(Position::nnueConfig::on);

	auto& nnue = pos.nnue();
	if(!nnue->load("internal")) {
		std::cout<<"error loading "<<"internal"<<std::endl;
		return 1;
	}

	//----------------------------------
	const auto start = std::chrono::high_resolution_clock::now();
	std::vector<std::future<void>> helperThread;


	stream.open("fen.data", std::ios_base::app);
	TxtSaver fs(1, stream);
	for(int i = 0; i < TunerParameters::parallelGames; ++i){
		helperThread.emplace_back(std::async(std::launch::async, worker));
	}

	while(!stop) {
		int stoppedCount = 0;
		for(auto &t : helperThread) {
			stoppedCount += (t.wait_for(0ms) == std::future_status::ready);
		}
		if(stoppedCount == TunerParameters::parallelGames)  {
			break;
		}
		std::this_thread::sleep_for(10000ms);
		const auto end = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double, std::milli> elapsed = end - start;
		std::cout<<std::chrono::duration_cast<std::chrono::minutes>(elapsed).count()<<" saved "<<fs.getSavedPosition()/1000000.0<<"M positions "<< fs.getSavedPosition()/std::chrono::duration_cast<std::chrono::seconds>(elapsed).count()<<" positions per second"<<std::endl;

	}


	stream.close();
	std::cout<<"saved "<<fs.getSavedPosition()<<" positions"<<std::endl;

	return 0;
}
