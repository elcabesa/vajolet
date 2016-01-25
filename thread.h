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



struct timeManagementStruct
{
	volatile unsigned long allocatedTime;
	volatile unsigned long minSearchTime;
	volatile unsigned long maxSearchTime;
	volatile unsigned int depth;
	volatile unsigned int singularRootMoveCount;
	volatile unsigned int resolution;
	volatile bool idLoopIterationFinished;

};



class my_thread
{

	my_thread()
	{
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
	search src;

	static long long lastHasfullMessage;

	void initThreads();

	void timerThread();
	void searchThread();
	void manageNewSearch();
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

	static timeManagementStruct timeMan;

	~my_thread()
	{
		quitThreads();
	}
	void startThinking(Position * p, searchLimits& l)
	{
		src.stop = true;
		lastHasfullMessage = 0;

		while(startThink){}

		if(!startThink)
		{
			std::lock_guard<std::mutex> lk(searchMutex);
			src.limits = l;
			src.pos = *p;
			startThink = true;
			searchCond.notify_one();
		}
	}

	void stopThinking()
	{
		src.stop = true;
		src.stopPonder();
	}

	void ponderHit()
	{
		src.stopPonder();
	}

};




#endif /* THREAD_H_ */
