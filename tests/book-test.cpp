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

//#include <iostream>
#include "gtest/gtest.h"
#include "./../book.h"
//#include "./../command.h"
#include "./../move.h"
#include "./../position.h"


TEST(bookTest, probe){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	PolyglotBook b;
	Move m = b.probe(p, true);
	//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
	EXPECT_EQ(m, Move(E2,E4));
	
}

TEST(bookTest, probeFail){
	Position p;
	p.setupFromFen("rnbqkbnr/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	PolyglotBook b;
	Move m = b.probe(p, true);
	//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
	EXPECT_EQ(m, Move::NOMOVE);
}
	
