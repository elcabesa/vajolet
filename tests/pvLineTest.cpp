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
#include "./../pvLine.h"

TEST(PVline, constructor)
{
	PVline p;
	ASSERT_EQ( p.size(), 0 );
	ASSERT_EQ( p.getMove(0), Move::NOMOVE );
	ASSERT_EQ( p.getMove(3), Move::NOMOVE );
	
}


TEST(PVline, set)
{
	PVline p;
	
	p.set( Move( E2, E4) );
	ASSERT_EQ( p.size(), 1 );
	ASSERT_EQ( p.getMove(0), Move( E2, E4) );
	ASSERT_EQ( p.getMove(1), Move::NOMOVE );
	
	p.set( Move( E7, E5) );
	ASSERT_EQ( p.size(), 1 );
	ASSERT_EQ( p.getMove(0), Move( E7, E5) );
	ASSERT_EQ( p.getMove(1), Move::NOMOVE );
	
	p.set( Move( D2, D4) );
	ASSERT_EQ( p.size(), 1 );
	ASSERT_EQ( p.getMove(0), Move( D2, D4) );
	ASSERT_EQ( p.getMove(1), Move::NOMOVE );

}

TEST(PVline, appendNewPvLine)
{
	PVline p1;
	p1.set( Move( D7, D5) );
	ASSERT_EQ( p1.size(), 1 );
	
	PVline p2;
	p2.appendNewPvLine( Move( D2, D4), p1 );
	ASSERT_EQ( p2.size(), 2 );
	
	PVline p3;
	p3.appendNewPvLine( Move( E7, E5), p2 );
	ASSERT_EQ( p3.size(), 3 );
	
	PVline res;
	res.appendNewPvLine( Move( E2, E4), p3);
	
	ASSERT_EQ( res.size(), 4 );
	ASSERT_EQ( res.getMove(0), Move( E2, E4) );
	ASSERT_EQ( res.getMove(1), Move( E7, E5) );
	ASSERT_EQ( res.getMove(2), Move( D2, D4) );
	ASSERT_EQ( res.getMove(3), Move( D7, D5) );
	ASSERT_EQ( res.getMove(4), Move::NOMOVE );
	
	ASSERT_EQ( p1.size(), 0 );
	ASSERT_EQ( p2.size(), 0 );
	ASSERT_EQ( p3.size(), 0 );
	
	res.clear();
	ASSERT_EQ( res.size(), 0 );
	ASSERT_EQ( res.getMove(0), Move::NOMOVE );
	ASSERT_EQ( res.getMove(1), Move::NOMOVE );
	ASSERT_EQ( res.getMove(2), Move::NOMOVE );
	ASSERT_EQ( res.getMove(3), Move::NOMOVE );
	ASSERT_EQ( res.getMove(4), Move::NOMOVE );

}

TEST(PVline, iterator)
{
	PVline p1;
	p1.set( Move( D7, D5) );
	ASSERT_EQ( p1.size(), 1 );
	
	PVline p2;
	p2.appendNewPvLine( Move( D2, D4), p1 );
	ASSERT_EQ( p2.size(), 2 );
	
	PVline p3;
	p3.appendNewPvLine( Move( E7, E5), p2 );
	ASSERT_EQ( p3.size(), 3 );
	
	PVline res;
	res.appendNewPvLine( Move( E2, E4), p3);
	
	unsigned count = 0;
	for( auto m: res )
	{
		switch(count)
		{
			case 0:
				ASSERT_EQ( m, Move( E2, E4)  );
				break;
			case 1:
				ASSERT_EQ( m, Move( E7, E5) );
				break;
			case 2:
				ASSERT_EQ( m, Move( D2, D4) );
				break;
			case 3:
				ASSERT_EQ( m, Move( D7, D5) );
				break;
			default:
				ASSERT_EQ( m, Move::NOMOVE );
		}
		
		++count;
	}
}
