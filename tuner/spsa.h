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
#ifndef SPSA_H_
#define SPSA_H_

#include <random>
#include <vector>
#include "parameters.h"
#include "tournament.h"

class Player;


class SPSA {
public:
	SPSA();
	void run();

private:
	static constexpr double alpha = 0.602;
	static constexpr double gamma = 0.101;
	static constexpr int N = 50000;
	static constexpr double A = 5000;
	
	struct runtimeVars
	{
		double value;
		double a;
		double c;
		double r;
		int delta;
	};

	struct variable
	{
		variable(std::string n, int SearchParameters::*p, const double sv, const double minv, const double maxv, const double cE, const double rE):
		name(n),
		par(p),
		startValue(sv),
		minValue(minv),
		maxValue(maxv),
		cEnd(cE),
		rEnd(rE),
		run({})
		{}
		const std::string name;
		int SearchParameters::*par;
		const double startValue;
		const double minValue;
		const double maxValue;
		const double cEnd;
		const double rEnd;
		runtimeVars run;
	};
	
	std::vector<variable> _pars;
	
	void _populateParameters();
	void _generateParamters(Player& p1, Player& p2, int k);
	void _printParameters(Player& p1, Player& p2);
	TournamentResult _runTournament(Player& p1, Player& p2, int k);
	void _updateParamters(TournamentResult& r);
	
	std::mt19937 _gen;
};

#endif