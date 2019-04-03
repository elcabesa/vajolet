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
#include "./../pvLineFollower.h"


TEST(PVlineFollower, getNextMove)
{
	PVline p1;
	p1.set( Move(D7, D5) );
	
	PVline p2;
	p2.appendNewPvLine( Move(D2, D4), p1 );
	
	PVline p3;
	p3.appendNewPvLine( Move(E7, E5), p2 );
	
	PVline res;
	res.appendNewPvLine( Move(E2, E4), p3);

	PVlineFollower pvf;
	pvf.setPVline(res);
	
	pvf.restart();
	
	Move m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(0, m), true );
	ASSERT_EQ( m, Move(E2, E4));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(1, m), true );
	ASSERT_EQ( m, Move(E7, E5));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(2, m), true );
	ASSERT_EQ( m, Move(D2, D4));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(3, m), true );
	ASSERT_EQ( m, Move(D7, D5));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(4, m), false );
	ASSERT_EQ( m, Move::NOMOVE);
	
	pvf.restart();
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(0, m), true );
	ASSERT_EQ( m, Move(E2, E4));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(1, m), true );
	ASSERT_EQ( m, Move(E7, E5));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(2, m), true );
	ASSERT_EQ( m, Move(D2, D4));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(3, m), true );
	ASSERT_EQ( m, Move(D7, D5));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(4, m), false );
	ASSERT_EQ( m, Move::NOMOVE);
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(2, m), false );
	ASSERT_EQ( m, Move::NOMOVE);
	
	
}

TEST(PVlineFollower, getNextMove2)
{
	PVline p1;
	p1.set( Move(D7, D5) );
	
	PVline p2;
	p2.appendNewPvLine( Move(D2, D4), p1 );
	
	PVline p3;
	p3.appendNewPvLine( Move(E7, E5), p2 );
	
	PVline res;
	res.appendNewPvLine( Move(E2, E4), p3);

	PVlineFollower pvf;
	pvf.setPVline(res);
	
	pvf.restart();
	
	Move m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(0, m), true );
	ASSERT_EQ( m, Move(E2, E4));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(1, m), true );
	ASSERT_EQ( m, Move(E7, E5));
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(1, m), false );
	ASSERT_EQ( m, Move::NOMOVE);
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(3, m), false );
	ASSERT_EQ( m, Move::NOMOVE);
	
	m = Move::NOMOVE;
	ASSERT_EQ( pvf.getNextMove(4, m), false );
	ASSERT_EQ( m, Move::NOMOVE);
	

}

TEST(PVlineFollower, clean)
{
	PVline p1;
	p1.set( Move( D7, D5) );
	
	PVline p2;
	p2.appendNewPvLine( Move( D2, D4), p1 );
	
	PVline p3;
	p3.appendNewPvLine( Move( E7, E5), p2 );
	
	PVline res;
	res.appendNewPvLine( Move( E2, E4), p3);

	PVlineFollower pvf;
	pvf.setPVline(res);
	pvf.clear();
	
	Move m= Move::NOMOVE;
	for( unsigned int i = 0; i <20 ;++i)
	{
		ASSERT_EQ( pvf.getNextMove(i, m), false );
		ASSERT_EQ( m, Move::NOMOVE);
	}
}

TEST(PVlineFollower, empty)
{
	PVlineFollower pvf;
	Move m= Move::NOMOVE;
	for( unsigned int i = 0; i <20 ;++i)
	{
		ASSERT_EQ( pvf.getNextMove(i, m), false );
		ASSERT_EQ( m, Move::NOMOVE);
	}

}

