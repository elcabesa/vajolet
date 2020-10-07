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
#include "uciParameters.h"

unsigned int uciParameters::threads = 1;
unsigned int uciParameters::multiPVLines = 1;
bool uciParameters::useOwnBook = true;
bool uciParameters::bestMoveBook = false;
bool uciParameters::showCurrentLine = false;
std::string uciParameters::SyzygyPath = "<empty>";
unsigned int uciParameters::SyzygyProbeDepth = 1;
bool uciParameters::Syzygy50MoveRule =  true;
bool uciParameters::Ponder;
bool uciParameters::Chess960 = false;
bool uciParameters::perftUseHash = false;
bool uciParameters::limitStrength = false;
unsigned int uciParameters::engineLevel = 20;
bool uciParameters::useNnue = true; 
std::string uciParameters::nnueFile = "nnue.par";


