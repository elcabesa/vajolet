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

#include <iostream>

#include "command.h"
#include "game.h"
#include "position.h"

void Game::CreateNewGame(void)
{
	_positions.clear();
}

void Game::insertNewMoves(Position &pos)
{
	unsigned int actualPosition = _positions.size();
	for(unsigned int i = actualPosition; i < pos.getStateSize(); i++)// todo usare iteratore dello stato
	{
		GamePosition p;
		p.key = pos.getState(i).getKey();
		p.m = pos.getState(i).getCurrentMove();
		p.depth = 0;
		_positions.push_back(p);
	}
}

void Game::savePV(PVline PV,unsigned int depth, Score alpha, Score beta)
{
	_positions.back().PV = PV;
	_positions.back().depth = depth;
	_positions.back().alpha = alpha;
	_positions.back().beta = beta;
}


void Game::printGamesInfo()
{
	for(auto p : _positions)
	{
		if( p.m )
		{
			std::cout<<"Move: "<<UciManager::displayUci(p.m)<<"  PV:";
			for( auto m : p.PV )
			{
				std::cout<<UciManager::displayUci(m)<<" ";
			}

		}
		std::cout<<std::endl;
	}

}

bool Game::isNewGame(const Position &pos) const
{
	if( _positions.size() == 0 || pos.getStateSize() < _positions.size())
	{
		//printGamesInfo();
		return true;
	}

	unsigned int n = 0;
	for(auto p : _positions)
	{
		if(pos.getState(n).getKey() != p.key)
		{
			//printGamesInfo();
			return true;
		}
		n++;

	}
	return false;
}

bool Game::isPonderRight() const
{
	if( _positions.size() > 2)
	{
		GamePosition previous =*(_positions.end()-3);
		if(previous.PV.size()>=1 && previous.PV.getMove(1) == _positions.back().m)
		{
			return true;
		}

	}
	return false;
}

Game::GamePosition Game::getNewSearchParameters() const
{
	GamePosition previous =*(_positions.end()-3);
	return previous;
}