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

#include <mutex>

#include "io.h"

using namespace std;


/*! \brief definition of the << operator to be able to use it in a multithread context

	\author STOCKFISH
	\version 1.0
	\date 14/10/2013
*/

std::ostream& operator<<(std::ostream& os, SyncCout sc) {
	static std::mutex m;
	if (sc == SyncCout::io_lock)
		m.lock();

	if (sc == SyncCout::io_unlock)
		m.unlock();

	return os;
}
