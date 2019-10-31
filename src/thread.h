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

#include <condition_variable>
#include <memory>


class Position;
class timeManagement;
class SearchLimits;
class SearchResult;


class my_thread
{
private:
	explicit my_thread();
	~my_thread();
	my_thread(const my_thread&) = delete;
	my_thread& operator=(const my_thread&) = delete;
	my_thread(const my_thread&&) = delete;
	my_thread& operator=(const my_thread&&) = delete;
	
	class impl;
	std::unique_ptr<impl> pimpl;
public :

	static my_thread& getInstance()
	{
		static my_thread pInstance;
		return pInstance;
	}

	void startThinking( const Position& p, SearchLimits& l);
	void stopThinking();
	void ponderHit();
	timeManagement& getTimeMan();
	std::condition_variable& finished();
	const SearchResult& getResult() const;
	void setMute(bool mute);
};
#endif /* THREAD_H_ */
