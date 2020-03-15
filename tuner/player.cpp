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

#include <cmath>
#include <string>

#include "elo.h"
#include "player.h"
#include "transposition.h"


Player::Player(std::string name): _name(name){
	_thr.setMute(true);
	_thr.getTT().setSize(1);
}

const SearchParameters& Player::getSearchParametersConst() const { return _sp; }
SearchParameters& Player::getSearchParameters() { return _sp; }

void Player::insertResult(int res) {
	_win += (res == 1);
	_lost += (res == -1);
	_draw += (res == 0);
	_unknown += (res == -2);
}

std::string Player::print() const {
	std::string s;
	Elo elo(_win, _lost, _draw);
	s += std::to_string(_win);
	s += "/";
	s += std::to_string(_lost);
	s += "/";
	s += std::to_string(_draw);
	s += " ";
	s += std::to_string(_unknown);
	s += " ";
	s += std::to_string(elo.diff());
	s += " elo +-";
	s += std::to_string(elo.errorMargin());
	s += " elo LOS(";
	s += std::to_string(elo.LOS());
	s += ")";
	return s;
}

const std::string& Player::getName() const {
	return _name;
}

double Player::pointRatio() const {
	return 	Elo(_win, _lost, _draw).pointRatio();
}

SearchResult Player::doSearch(const Position& p, SearchLimits& sl)
{
	_thr.getSearchParameters() = getSearchParametersConst();
	return _thr.synchronousSearch(p, sl);
}