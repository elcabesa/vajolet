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

#include <random>
#include <ctime>
#include "search.h"
#include "movegen.h"

std::mt19937_64 rnd;
std::uniform_int_distribution<uint64_t> uint_dist;



void search::startThinking(Position & p){
	/*************************************************
	 *	first of all check the number of legal moves
	 *	if there is only 1 moves do it
	 *	if there is 0 legal moves return null move
	 *************************************************/
	Movegen mg;
	mg.generateMoves<Movegen::allMg>(p);
	if(mg.getGeneratedMoveNumber()==0){
		sync_cout<<"0000"<<sync_endl;
		return;
	}
	if(mg.getGeneratedMoveNumber()==1){
		sync_cout<<p.displayUci(mg.getGeneratedMove(0))<<sync_endl;
		return;
	}


	sync_cout<<p.displayUci(mg.getGeneratedMove(uint_dist(rnd)%mg.getGeneratedMoveNumber()))<<sync_endl;


}


