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

#include "libchess.h"
#include "position.h"
#include "search.h"
#include "syzygy/syzygy.h"


void libChessInit()
{
	initData();
	HashKey::init();
	Position::initScoreValues();
	Movegen::initMovegenConstant();
	Search::initSearchParameters();
	Position::initMaterialKeys();
	Syzygy::getInstance();
	
	Syzygy::getInstance().setPath("C:/Users/elcab/Downloads/syzygy");
	std::cout<<"TBSIZE: "<<Syzygy::getInstance().getSize()<<std::endl;
	std::cout<<"CARDINALITY: "<<Syzygy::getInstance().getMaxCardinality()<<std::endl;
	Position pos;
	pos.setupFromFen("8/b2B4/8/8/6R1/5K2/7k/8 w - -");
	ProbeState result;
	WDLScore score =Syzygy::getInstance().probeWdl(pos, result);
	std::cout<<"result: "<<score<<std::endl;
	
	
}