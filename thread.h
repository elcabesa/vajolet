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
#include "transposition.h"



typedef struct timeManagement{
	volatile unsigned long allocatedTime;
	volatile unsigned long minSearchTime;
	volatile unsigned long maxSearchTime;
	volatile unsigned int depth;
	volatile unsigned int singularRootMoveCount;
	volatile unsigned int resolution;
	volatile bool idLoopIterationFinished;
} timeManagementStruct;



class my_thread{

	my_thread(){
		initThreads();
	};

	static my_thread * pInstance;


	volatile static bool quit;
	volatile static bool startThink;
	std::thread timer;
	std::thread searcher;
	std::mutex searchMutex;
	std::condition_variable searchCond;
	std::condition_variable timerCond;
	Position *pos;
	search src;
	searchLimits limits;

	static long long startTime;
	static long long lastHasfullMessage;

	void initThreads();

	void timerThread();
	void searchThread();
public :
	void quitThreads();

	static std::mutex  _mutex;

	static my_thread* getInstance()
	{
		if (!pInstance)
		{
			std::lock_guard<std::mutex> lock(_mutex);

			if (!pInstance)
			{
				my_thread * temp = new my_thread;
			    pInstance = temp;
			}
		}

		return pInstance;
	}
	unsigned long long getVisitedNodes(){
		return src.getVisitedNodes();
	}
	static timeManagementStruct timeMan;
	~my_thread(){
		quitThreads();
	}
	void startThinking(Position * p,searchLimits& l){

		src.signals.stop=true;
		lastHasfullMessage=0;


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
		src.limits.ponder=false;
		limits.ponder=false;
	}

	void ponderHit(){
		startTime=std::chrono::duration_cast<std::chrono::milliseconds >(std::chrono::steady_clock::now().time_since_epoch()).count();
		limits.ponder=false;
		src.limits.ponder=false;
	}


};




#endif /* THREAD_H_ */
