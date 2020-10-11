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

#include "tunerPars.h"

int TunerParameters::parallelGames = 22;
int TunerParameters::gameNumber = 20;
unsigned int TunerParameters::randomMoveEveryXPly = 10;
	
float TunerParameters::gameTime = 10;
float TunerParameters::gameTimeIncrement = 0.1;

unsigned int TunerParameters::minDepth = 4;
unsigned int TunerParameters::MaxDepth = 6;

