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

#ifndef THREAD_H_
#define THREAD_H_

#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>
#include "position.h"
#include "search.h"



typedef struct timeManagement{
	unsigned long allocatedTime;
	unsigned long minSearchTime;
	unsigned long maxSearchTime;
	unsigned int depth;
	bool idLoopIterationFinished;
	unsigned int singularRootMoveCount;
	bool idLoopRequestToExtend;
} timeManagementStruct;



class my_thread{



	volatile static bool quit;
	volatile static bool startThink;
	std::thread timer;
	std::thread searcher;
	static std::mutex searchMutex;
	static std::condition_variable searchCond;
	static Position *pos;
	static search src;
	static searchLimits limits;

	static unsigned long startTime;

	void initThreads();
	void quitThreads();
	static void timerThread();
	static void searchThread();
public :
	static timeManagementStruct timeMan;
	my_thread(){
		initThreads();
	};
	~my_thread(){
		quitThreads();
	}
	void startTinking(Position * p,searchLimits& l){

		src.signals.stop=true;

		//sync_cout<<"STOOOOOOPPPPE"<<sync_endl;
		while(startThink){
			//sync_cout<<"attesa"<<sync_endl;
		}
		//sync_cout<<"fine attesa"<<sync_endl;

		if(!startThink){
			std::lock_guard<std::mutex> lk(searchMutex);
			limits=l;
			pos=p;
			startThink=true;
			searchCond.notify_one();
		}

	}

	void stopThinking(){
		//sync_cout<<"received stop"<<sync_endl;
		src.signals.stop=true;
	}

	void ponderHit(){
		startTime=std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		limits.ponder=false;
	}


};




#endif /* THREAD_H_ */
