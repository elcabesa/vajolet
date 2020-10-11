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
#include <iostream>
#include <random>
#include <string>

#include "elo.h"
#include "position.h"
#include "player.h"
#include <search.h>
#include "transposition.h"
#include "searchResult.h"
#include "timeManagement.h"
#include "tunerPars.h"


Player::Player(std::string name): _name(name),_src(_st, _sl, _tt, UciOutput::create(UciOutput::type::mute))
{
	//_thr.setMute(true);
	//_thr.getTT().setSize(64);
	_sl.setDepth(4);
	_tt.setSize(64);
}

const SearchParameters& Player::getSearchParametersConst() const { return _sp; }
SearchParameters& Player::getSearchParameters() { return _sp; }

const EvalParameters& Player::getEvalParametersConst() const { return _ep; }
EvalParameters& Player::getEvalParameters() { return _ep; }

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

SearchResult Player::doSearch(const Position& p, SearchLimits&)
{
	/*
	_thr.getSearchParameters() = getSearchParametersConst();
	Position pos(nullptr, Position::pawnHash::off, _ep);
	pos.setupFromFen(p.getFen());
	return _thr.synchronousSearch(pos, sl);*/
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distrib(TunerParameters::minDepth, TunerParameters::MaxDepth);
	_src.getPosition() = p;
	int depth = distrib(gen);
	_sl.setDepth(depth);
	return _src.manageNewSearch(*timeManagement::create(_sl, p.getNextTurn()));
}
