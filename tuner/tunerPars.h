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
#ifndef PARAMETER_H_
#define PARAMETER_H_

//todo read data from a json
class TunerParameters {
public:
	static constexpr int gameNumber = 25000;
	
	static constexpr float gameTime = 5;
	static constexpr float gameTimeIncrement = 0.05;
};

#endif
