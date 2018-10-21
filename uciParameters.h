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
#ifndef UCI_PARAMETERS_H_
#define UCI_PARAMETERS_H_

#include <string>

class uciParameters
{
public:
	static unsigned int threads;
	static unsigned int multiPVLines;
	static bool useOwnBook;
	static bool bestMoveBook;
	static bool showCurrentLine;
	static std::string SyzygyPath;
	static unsigned int SyzygyProbeDepth;
	static bool Syzygy50MoveRule;
};

#endif
