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

#include <algorithm>
//#include <iostream>
#include "gtest/gtest.h"
#include "./../book.h"
//#include "./../command.h"
#include "./../move.h"
#include "./../position.h"


TEST(bookTest, probeBest){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, true);
		//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_EQ(m, Move(E2,E4));
	}	
}

TEST(bookTest, probeBest1){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	p.doMove(Move(E2,E4));
	p.doMove(Move(C7,C6));
	p.doMove(Move(D2,D4));
	p.doMove(Move(D7,D5));
	p.doMove(Move(B1,C3));
	p.doMove(Move(D5,E4));
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, false);
		//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_EQ(m, Move(C3,E4));
	}	
}

TEST(bookTest, probeRandom1){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	p.doMove(Move(E2,E4));
	p.doMove(Move(C7,C6));
	p.doMove(Move(D2,D4));
	p.doMove(Move(D7,D5));
	p.doMove(Move(B1,C3));
	p.doMove(Move(D5,E4));
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, false);
		//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_EQ(m, Move(C3,E4));
	}	
}

TEST(bookTest, probeCastle){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
	p.doMove(Move(E2,E4));
	p.doMove(Move(E7,E5));
	p.doMove(Move(G1,F3));
	p.doMove(Move(B8,C6));
	p.doMove(Move(F1,C4));
	p.doMove(Move(F8,C5));
	
	std::vector<Move> v ={ Move(C2,C3), Move(E1,G1,Move::fcastle)};
	
	PolyglotBook b;
	for (int i = 0;i<100;++i) {
		Move m = b.probe(p, false);
		auto res = std::find(v.begin(), v.end(), m);
		//if( res != v.end())std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_NE(res, v.end());
	}	
}

TEST(bookTest, probeRandom){
	Position p;
	p.setupFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	std::vector<Move> v ={ Move(E7,E5), Move(C7,C5), Move(E7,E6), Move(C7,C6)};
	p.doMove(Move(E2,E4));
	
	PolyglotBook b;
	for (int i = 0;i<1000;++i) {
		Move m = b.probe(p, false);
		auto res = std::find(v.begin(), v.end(), m);
		//if( res == v.end())std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
		ASSERT_NE(res, v.end());
	}	
}

TEST(bookTest, probeFail){
	Position p;
	p.setupFromFen("rnbqkbnr/8/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	
	PolyglotBook b;
	Move m = b.probe(p, true);
	//std::cout<<UciManager::getInstance().displayUci(m)<<std::endl;
	ASSERT_EQ(m, Move::NOMOVE);
}

// todo promotion and enpassant?

