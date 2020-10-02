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

#include <fstream>
#include <iostream>
#include <sstream>

#include "book.h"
#include "movepicker.h"
#include "position.h"
#include "PGNGame.h"
#include "PGNMoveList.h"
#include "PGNException.h"
#include "uciOutput.h"

Book::Book(const std::string& file)
{
	std::ifstream pgnfile(file);
	pgnfile >> _games;
	_itr = _games.begin();
	
}

Move Book::_moveFromPgn(const Position& pos,const  std::string& str){
	// idea from stockfish, we generate all the legal moves and return the legal moves with the same UCI string
	Move m;
	MovePicker mp(pos);
	//std::cout<<str<<" "<<std::endl;
	while( ( m = mp.getNextMove() ) )
	{
		if(str == UciOutput::displayMove(pos, m))
		{
			//std::cout<<"found"<<std::endl;
			return m;
		}
	}
	std::cout<<"not found"<<std::endl;
	exit(-1);
	return Move::NOMOVE;
}

std::list<Move> Book::getLine()
{
	pgn::Game game;
	std::list<Move> lst;
	{
		std::lock_guard<std::mutex> lck(_mtx);
		game = *_itr;
		++_itr;
		if(_itr == _games.end()) {
			_itr = _games.begin();
		}
	}
	pgn::MoveList movelist = game.moves();

	Position p(Position::pawnHash::off);
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	// now let's iterate trough the list of moves and print each one to stdout
	for (pgn::MoveList::iterator itr2 = movelist.begin(); itr2 != movelist.end(); itr2++)
	{
		//std::cout << itr2->white() <<" "<< itr2->black() << " ";
		if(itr2->white().valid())
		{
			std::ostringstream stream;
			stream << itr2->white();
			Move m = _moveFromPgn(p, stream.str());
			p.doMove(m);
			lst.push_back(m);
		}
		if(itr2->black().valid())
		{
			std::ostringstream stream;
			stream << itr2->black();
			Move m = _moveFromPgn(p, stream.str());
			p.doMove(m);
			lst.push_back(m);
		}
		
	}
	
	return lst;
	
}
