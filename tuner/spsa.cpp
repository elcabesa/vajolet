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

#include <string>
#include <vector>

#include "player.h"
#include "spsa.h"
#include "tournament.h"

SPSA::SPSA():_gen((std::random_device())()) {
	std::cout<<"SPSA"<<std::endl;
}

void SPSA::_populateParameters()
{
	_pars.emplace_back("razorMargin", &SearchParameters::razorMargin, 20000, 0, 40000, 5000, 0.0020);
	_pars.emplace_back("razorDepth", &SearchParameters::razorMarginDepth, 64, 0, 128, 15, 0.0020);
	//_pars.emplace_back("razorMarginCut", &SearchParameters::razorMarginCut, 20000, 0, 40000, 5000, 0.0020);
	
	_pars.emplace_back("staticNullMovePruningDepth", &SearchParameters::staticNullMovePruningDepth, 128, 0, 256, 40, 0.0020);
	_pars.emplace_back("staticNullMovePruningValue", &SearchParameters::staticNullMovePruningValue, 375, 0, 500, 50, 0.0020);
	_pars.emplace_back("staticNullMovePruningImprovingBonus", &SearchParameters::staticNullMovePruningImprovingBonus, 2000, 0, 4000, 500, 0.0020);

	_pars.emplace_back("nullMovePruningDepth", &SearchParameters::nullMovePruningDepth, 16, 0, 32, 4, 0.0020);
	_pars.emplace_back("nullMovePruningReduction", &SearchParameters::nullMovePruningReduction, 48, 0, 96, 12, 0.0020);
	_pars.emplace_back("nullMovePruningBonusThreshold", &SearchParameters::nullMovePruningBonusThreshold, 10000, 0, 20000, 2000, 0.0020);
	_pars.emplace_back("nullMovePruningBonusAdditionalRed", &SearchParameters::nullMovePruningBonusAdditionalRed, 16, 0, 32, 4, 0.0020);
	_pars.emplace_back("nullMovePruningVerificationDepth", &SearchParameters::nullMovePruningVerificationDepth, 192, 0, 400, 50, 0.0020);
	
	_pars.emplace_back("probCutDepth", &SearchParameters::probCutDepth, 80, 0, 160, 20, 0.0020);
	_pars.emplace_back("probCutDelta", &SearchParameters::probCutDelta, 8000, 0, 16000, 2000, 0.0020);
	_pars.emplace_back("probCutDepthRed", &SearchParameters::probCutDepthRed, 48, 0, 100, 10, 0.0020);
	
	_pars.emplace_back("iidDepthPv", &SearchParameters::iidDepthPv, 80, 0, 160, 20, 0.0020);
	_pars.emplace_back("iidDepthNonPv", &SearchParameters::iidDepthNonPv, 128, 0, 256, 32, 0.0020);
	_pars.emplace_back("iidStaticEvalBonus", &SearchParameters::iidStaticEvalBonus, 10000, 0, 20000, 2000, 0.0020);
	_pars.emplace_back("iidDepthRed", &SearchParameters::iidDepthRed, 32, 0, 64, 8, 0.0020);
	_pars.emplace_back("iidDepthRedFactor", &SearchParameters::iidDepthRedFactor, 4, 0, 8, 1, 0.0020);
	
	_pars.emplace_back("singularExpressionPVDepth", &SearchParameters::singularExpressionPVDepth, 96, 0, 200, 20, 0.0020);
	_pars.emplace_back("singularExpressionNonPVDepth", &SearchParameters::singularExpressionNonPVDepth, 128, 0, 256, 25, 0.0020);
	_pars.emplace_back("singularExpressionTtDepth", &SearchParameters::singularExpressionTtDepth, 48, 0, 96, 10, 0.0020);
	
	for( auto& par: _pars) {
		par.run.value = par.startValue;
	}
}

void SPSA::_generateParamters(Player& p1, Player& p2, int k)
{
	std::bernoulli_distribution d(0.5);
	
	for( auto& par: _pars) {
		std::cout<<"\tCalculating "<<par.name<<std::endl;
		double _c = par.cEnd * std::pow(N, gamma);
		double _aEnd = par.rEnd * std::pow(par.cEnd, 2.0);
		double _a = _aEnd * std::pow(A + N, alpha);
		std::cout<<"\t\tc:"<<_c<<" aEnd:"<<_aEnd<<" a:"<<_a<<std::endl;
		
		par.run.a = _a / std::pow(A + k, alpha);
		par.run.c = _c / std::pow(k, gamma);
		par.run.r = par.run.a / std::pow(par.run.c, 2.0);
		par.run.delta = d(_gen) ? 1 : -1;
		std::cout<<"\t\ta:"<<par.run.a<<" c:"<<par.run.c<<" r:"<<par.run.r<<" delta:"<<par.run.delta<<std::endl;
		p1.getSearchParameters().*(par.par) = std::max(std::min(par.run.value + par.run.c * par.run.delta, par.maxValue), par.minValue);
		p2.getSearchParameters().*(par.par) = std::max(std::min(par.run.value - par.run.c * par.run.delta, par.maxValue), par.minValue);
	}
}

TournamentResult SPSA::_runTournament(Player& p1, Player& p2, int k)
{
	Tournament t("tournament" + std::to_string(k)+".pgn", p1, p2);
	auto res = t.play();
	std::cout<<"Tournament Result: "<<static_cast<int>(res)<<std::endl;
	return res;
}

void SPSA::_printParameters(Player& p1, Player& p2)
{
	std::cout<<"----------PARAMETERS--------"<<std::endl;	
	for(auto& par: _pars) {
		std::cout<<par.name<<" "<<p1.getSearchParameters().*(par.par)<<" "<<p2.getSearchParameters().*(par.par)<<std::endl;
	}
}

void SPSA::_updateParamters(TournamentResult& res)
{
	for( auto& par: _pars) {
		par.run.value = std::max(std::min(par.run.value + par.run.r * par.run.c * static_cast<int>(res) * par.run.delta, par.maxValue), par.minValue);
		std::cout<<par.name<<":"<<par.run.value<<std::endl;
	}
}

void SPSA::run() {	
	_populateParameters();

	for(int k = 1; k <= N; ++k) {
		std::cout<<"iteration "<<k<<std::endl;
		
		Player p1("p1");
		Player p2("p2");
		
		_generateParamters(p1, p2, k);
		_printParameters(p1, p2);
		auto res = _runTournament(p1, p2, k);
		_updateParamters(res);
	}

}