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
#ifndef PLAYER_H_
#define PLAYER_H_

#include <string>
#include "parameters.h"

class Player {
public:

	Player(std::string name);
	const SearchParameters& getSearchParametersConst() const;
	SearchParameters& getSearchParameters();
	void insertResult(int res);
	std::string print() const;
	const std::string& getName() const;
	
private:
	SearchParameters _sp;
	int _win = 0;
	int _lost = 0;
	int _draw = 0;
	int _unknown = 0;
	std::string _name;
	double _getWinProbability() const;
	double _getElo() const;
};

#endif /* PLAYER_H_ */