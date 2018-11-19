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

#include "gtest/gtest.h"
#include "./../MoveList.h"




namespace {
	
	TEST(MoveList,insert)
	{
		Move m1( E2 , E4);
		Move m2( G1 , D8);
		Move m3( D6 , C4);
		Move m4( E2 , E3);
		
		MoveList<30> ml;
		ml.insert(m1);
		ml.insert(m2);
		ml.insert(m3);
		ml.insert(m4);
		
		ASSERT_EQ( 4, ml.size() );
		
		ASSERT_EQ( m1, ml.get(0) );
		ASSERT_EQ( m2, ml.get(1) );
		ASSERT_EQ( m3, ml.get(2) );
		ASSERT_EQ( m4, ml.get(3) );
	}
	
	TEST(MoveList,reset)
	{
		Move m1( E2 , E4);
		Move m2( G1 , D8);
		Move m3( D6 , C4);
		Move m4( E2 , E3);
		
		MoveList<30> ml;
		ml.insert(m1);
		ml.insert(m2);
		ml.insert(m3);
		ml.insert(m4);
		ml.reset();
		
		ASSERT_EQ( 0, ml.size() );
		
	}
	
	TEST(MoveList,getNextMove)
	{
		Move m1( E2 , E4);
		Move m2( G1 , D8);
		Move m3( D6 , C4);
		Move m4( E2 , E3);
		
		MoveList<30> ml;
		ml.insert(m1);
		ml.insert(m2);
		ml.insert(m3);
		ml.insert(m4);
		
		ASSERT_EQ( 4, ml.size() );
		ASSERT_EQ( m1, ml.getNextMove() );
		ASSERT_EQ( m2, ml.getNextMove() );
		ASSERT_EQ( m3, ml.getNextMove() );
		ASSERT_EQ( m4, ml.getNextMove() );
		
	}
	
	TEST(MoveList,ignoreMove)
	{
		Move m1( E2 , E4);
		Move m2( G1 , D8);
		Move m3( D6 , C4);
		Move m4( E2 , E3);
		
		MoveList<30> ml;
		ml.insert(m1);
		ml.insert(m2);
		ml.insert(m3);
		ml.insert(m4);
		
		ml.ignoreMove(m3);
				
		ASSERT_EQ( 4, ml.size() );
		ASSERT_EQ( m2, ml.getNextMove() );
		ASSERT_EQ( m1, ml.getNextMove() );
		ASSERT_EQ( m4, ml.getNextMove() );
		
		ASSERT_FALSE( ml.getNextMove() );
		
	}
	
	TEST(MoveList,ignoreMove2)
	{
		Move m1( E2 , E4);
		Move m2( G1 , D8);
		Move m3( D6 , C4);
		Move m4( E2 , E3);
		
		Move m5( F1 , A8);
		
		MoveList<30> ml;
		ml.insert(m1);
		ml.insert(m2);
		ml.insert(m3);
		ml.insert(m4);
		
		ml.ignoreMove(m5);
				
		ASSERT_EQ( 4, ml.size() );
		ASSERT_EQ( m1, ml.getNextMove() );
		ASSERT_EQ( m2, ml.getNextMove() );
		ASSERT_EQ( m3, ml.getNextMove() );
		ASSERT_EQ( m4, ml.getNextMove() );
		
		ASSERT_FALSE( ml.getNextMove() );
		
	}
	
	TEST(MoveList,iterator1)
	{
		Move m1( E2 , E4);
		Move m2( G1 , D8);
		Move m3( D6 , C4);
		Move m4( E2 , E3);
		
		MoveList<30> ml;
		ml.insert(m1);
		ml.insert(m2);
		ml.insert(m3);
		ml.insert(m4);
		
		ASSERT_EQ( 4, ml.size() );
		
		auto it = ml.begin();
		ASSERT_EQ( m1, *it );
		++it;
		ASSERT_EQ( m2, *it );
		++it;
		ASSERT_EQ( m3, *it );
		++it;
		ASSERT_EQ( m4, *it );


	}
	
	TEST(MoveList,iterator2)
	{
		Move mm[] ={ Move(E2 , E4), Move( G1 , D8), Move( D6 , C4), Move( E2 , E3) };
		
		MoveList<30> ml;
		ml.insert(mm[0]);
		ml.insert(mm[1]);
		ml.insert(mm[2]);
		ml.insert(mm[3]);
		
		ASSERT_EQ( 4, ml.size() );
		unsigned int i= 0;
		for( auto it = ml.begin(); it != ml.end(); ++it)
		{
			ASSERT_EQ( mm[i], *it );
			++i;
		}
	}
	
	TEST(MoveList,iterator3)
	{
		Move mm[] ={ Move(E2 , E4), Move( G1 , D8), Move( D6 , C4), Move( E2 , E3) };
		
		MoveList<30> ml;
		ml.insert(mm[0]);
		ml.insert(mm[1]);
		ml.insert(mm[2]);
		ml.insert(mm[3]);
		
		ASSERT_EQ( 4, ml.size() );
		
		ml.getNextMove();
		
		unsigned int i= 1;
		
		for( auto it = ml.actualPosition(); it != ml.end(); ++it)
		{
			ASSERT_EQ( mm[i], *it );
			++i;
		}
	}
	
	TEST(MoveList,findNextBestMove)
	{
		Move mm[] ={ Move(E2 , E4), Move( G1 , D8), Move( D6 , C4), Move( E2 , E3) };
		Score ss[] ={ 10,600,25,950 };
		
		MoveList<30> ml;
		ml.insert(mm[0]);
		ml.insert(mm[1]);
		ml.insert(mm[2]);
		ml.insert(mm[3]);
		
		unsigned int i= 0;
		for( auto it = ml.actualPosition(); it != ml.end(); ++it)
		{
			(*it).setScore( ss[i] );
			++i;
		}
		
		ASSERT_EQ( mm[3], ml.findNextBestMove() );
		ASSERT_EQ( mm[1], ml.findNextBestMove() );
		ASSERT_EQ( mm[2], ml.findNextBestMove() );
		ASSERT_EQ( mm[0], ml.findNextBestMove() );
		ASSERT_FALSE( ml.findNextBestMove());
		
	}
	
	TEST(MoveList,emptyList)
	{
		
		MoveList<30> ml;
		
		ASSERT_EQ( 0, ml.size() );
		
		unsigned int i= 0;
		for( auto it = ml.actualPosition(); it != ml.end(); ++it)
		{
			++i;
		}
		
		ASSERT_EQ( 0, i );
		
		i = 0;
		for( auto it = ml.begin(); it != ml.end(); ++it)
		{
			++i;
		}
		ASSERT_EQ( 0, i );
		
		ASSERT_FALSE( ml.findNextBestMove());
		
		
	}
	
}
