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

#ifndef UCIMANAGERIMPL_H_
#define UCIMANAGERIMPL_H_

#include <istream>
#include <list>
#include <string>
#include <sstream>

#include "command.h"
#include "position.h"
#include "thread.h"

class UciOption;

class UciManager::impl
{
public:
	explicit impl();
	~impl();
	
	void uciLoop(std::istream& is);

	
private:
	/*! \brief array of char to create the fen string
		\author Marco Belli
		\version 1.0
		\date 27/10/2013
	*/
	
	void setTTSize(unsigned int size);
	void clearHash();
	void setTTPath( std::string s );
	std::string unusedVersion;
	unsigned int unusedSize;
	
	static const std::string _StartFEN;
	
	std::list<std::unique_ptr<UciOption>> _optionList;
	
	std::string _getProgramNameAndVersion();
	void _printNameAndVersionMessage();
	
	Move _moveFromUci(const Position& pos,const  std::string& str);
	void _doPerft(const unsigned int n);
	
	
	bool _printUciInfo(std::istringstream& is, my_thread &thr);
	bool _stop(std::istringstream& is, my_thread &thr);
	bool _uciNewGame(std::istringstream& is, my_thread &thr);
	bool _eval(std::istringstream& is, my_thread &thr);
	bool _display(std::istringstream& is, my_thread &thr);
	bool _position(std::istringstream& is, my_thread &thr);
	
	bool _go(std::istringstream& is, my_thread &thr);
	bool _setoption(std::istringstream& is, my_thread &thr);
	bool _readyOk(std::istringstream& is, my_thread &thr);
	bool _benchmark(std::istringstream& is, my_thread &thr);
	bool _ponderHit(std::istringstream& is, my_thread &thr);
	bool _perft(std::istringstream& is, my_thread &thr);
	bool _divide(std::istringstream& is, my_thread &thr);
	bool _quit(std::istringstream& is, my_thread &thr);
	Position _pos;
	my_thread thr;
};

#endif