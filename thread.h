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
#include <condition_variable>
#include "position.h"
#include "search.h"







class my_thread{

	static bool quit;
	static bool startThink;
	std::thread timer;
	std::thread searcher;
	static std::mutex searchMutex;
	static std::condition_variable searchCond;
	static Position *pos;
	static search src;
	static unsigned int searchTimeout;
	static unsigned int searchTimer;
	static searcLimits limits;

	void initThreads();
	void quitThreads();
	static void timerThread();
	static void searchThread();
public :
	my_thread(){
		initThreads();
	};
	~my_thread(){
		quitThreads();
	}
	void startTinking(Position * p,searcLimits& l){

		while(startThink){

		}
		if(!startThink){
			std::lock_guard<std::mutex> lk(searchMutex);
			limits=l;
			pos=p;
			startThink=true;
			searchCond.notify_one();
		}
	}

	void stopThinking(){
		src.signals.stop=true;
	}


};




#endif /* THREAD_H_ */
