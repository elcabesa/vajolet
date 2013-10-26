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


#include <thread>
#include <mutex>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "io.h"

using namespace std;

//--------------------------------------------------------------------
//	function implementation
//--------------------------------------------------------------------


/*! \brief stockfish fancy logging facility.

The trick here is to replace cin.rdbuf() and
cout.rdbuf() with two Tie objects that tie cin and cout to a file stream. We
can toggle the logging of std::cout and std:cin at runtime while preserving
usual i/o functionality and without changing a single line of code!
Idea from http://groups.google.com/group/comp.lang.c++/msg/1d941c0f26ea0d81

	\author STOCKFISH
	\version 1.0
	\date 21/10/2013
*/
struct Tie: public streambuf { // MSVC requires splitted streambuf for cin and cout

  Tie(streambuf* b, ofstream* f) : buf(b), file(f) {}

  int sync() { return file->rdbuf()->pubsync(), buf->pubsync(); }
  int overflow(int c) { return log(buf->sputc((char)c), "<< "); }
  int underflow() { return buf->sgetc(); }
  int uflow() { return log(buf->sbumpc(), ">> "); }

  streambuf* buf;
  ofstream* file;

  int log(int c, const char* prefix) {

    static int last = '\n';

    if (last == '\n')
        file->rdbuf()->sputn(prefix, 3);

    return last = file->rdbuf()->sputc((char)c);
  }
};


//----------------------------------------------------------------
//	class
//----------------------------------------------------------------
class Logger {

  Logger() : in(cin.rdbuf(), &file), out(cout.rdbuf(), &file) {}
 ~Logger() { start(false); }

  ofstream file;
  Tie in, out;

public:
  static void start(bool b) {

    static Logger l;

    if (b && !l.file.is_open())
    {
        l.file.open("io_log.txt", ifstream::out | ifstream::app);
        cin.rdbuf(&l.in);
        cout.rdbuf(&l.out);
    }
    else if (!b && l.file.is_open())
    {
        cout.rdbuf(l.out.buf);
        cin.rdbuf(l.in.buf);
        l.file.close();
    }
  }
};


/*! \brief function for starting outpup logging

	\author STOCKFISH
	\version 1.0
	\date 14/10/2013
*/
void start_logger(bool b) { Logger::start(b); }


/*! \brief definition of the << operator to be able to use it in a multithread context

	\author STOCKFISH
	\version 1.0
	\date 14/10/2013
*/
std::ostream& operator<<(std::ostream& os, SyncCout sc) {

	static mutex m;
	if (sc == io_lock)
		m.lock();

	if (sc == io_unlock)
		m.unlock();

	return os;
}
