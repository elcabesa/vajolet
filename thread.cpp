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

#include "thread.h"
#include "io.h"
#include "search.h"


unsigned int timeManagerInit(Position& pos, searcLimits& lim){
	if(pos.getActualState().nextMove){
		return lim.btime/40.0;
	}
	else{
		return lim.wtime/40.0;
	}

}


bool my_thread::quit=false;
bool my_thread::startThink=false;
Position *my_thread::pos;
search my_thread::src;
unsigned long my_thread::searchTimeout;
searcLimits my_thread::limits;

std::mutex my_thread::searchMutex;
std::condition_variable my_thread::searchCond;

unsigned long my_thread::startTime;



void my_thread::timerThread() {
	// TODO change time based on PV changing during the search.
	/*
	 * provare, usando la statistica e la define PRINT_PV_CHANGES a capire quante volte cambia la PV durante la ricerca,
	 * dargli uun peso in base al depth^2 e decidere in base a soglie o rapporti con i nodi etc se la posizione è calma o problematica
	*/
	std::mutex mutex;
	while (!quit)
	{
		std::unique_lock<std::mutex> lk(mutex);

		if (!quit){
			std::this_thread::sleep_for(std::chrono::milliseconds(5));


			if(std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count()-startTime>searchTimeout && !limits.infinite){
				src.signals.stop=true;
			}
		}
		lk.unlock();
  }
}

void my_thread::searchThread() {
	while (!quit)
	{
		std::unique_lock<std::mutex> lk(searchMutex);
		searchCond.wait(lk,[&]{return startThink||quit;});
		if(!quit){
			searchTimeout=timeManagerInit(*pos, limits);
			startTime=std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
			src.startThinking(*pos);
			startThink=false;

		}
		lk.unlock();
  }
}



void my_thread::initThreads(){
	timer=std::thread(timerThread);
	searcher=std::thread(searchThread);
}

void my_thread::quitThreads(){
	quit=true;
	searchCond.notify_one();
	timer.join();
	searcher.join();

}


