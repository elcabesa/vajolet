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
#include "./../move.h"



namespace {
	
	TEST(Move, constructor)
	{
		Move x;
		ASSERT_EQ( x.getPacked(), x.getPacked() );
	}
	
	TEST(Move, constructor2)
	{
		Move x(41233);
		ASSERT_EQ( 41233, x.getPacked() );
	}
	
	TEST(Move, constructor3)
	{
		Move x(41233);
		Move y(x);
		
		ASSERT_EQ( 41233, y.getPacked() );
	}
	
	TEST(Move, constructor4)
	{
		{
			Move x(tSquare::A2, tSquare::D6);
			
			ASSERT_EQ( tSquare::A2, x.getFrom());
			ASSERT_EQ( tSquare::D6, x.getTo());
			ASSERT_TRUE(x.isStandardMove());
			ASSERT_FALSE(x.isPromotionMove());
			ASSERT_FALSE(x.isCastleMove());
			ASSERT_FALSE(x.isEnPassantMove());
		}
		
		{
			Move y(tSquare::D2, tSquare::C1, Move::fenpassant);
			
			ASSERT_EQ( tSquare::D2, y.getFrom());
			ASSERT_EQ( tSquare::C1, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_FALSE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_TRUE(y.isEnPassantMove());
		}
		
		{
			Move y(tSquare::E1, tSquare::G1, Move::fcastle);
			
			ASSERT_EQ( tSquare::E1, y.getFrom());
			ASSERT_EQ( tSquare::G1, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_FALSE(y.isPromotionMove());
			ASSERT_TRUE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
		}
		
		{
			Move y(tSquare::E7, tSquare::E8, Move::fpromotion);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promQueen, y.getPromotionType());
		}
		
		{
			Move y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promQueen);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promQueen, y.getPromotionType());
		}
		
		{
			Move y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promRook);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promRook, y.getPromotionType());
		}
		
		{
			Move y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promBishop);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promBishop, y.getPromotionType());
		}
		
		{
			Move y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promKnight);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_FALSE(y.isStandardMove());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promKnight, y.getPromotionType());
		}
		
		
		
	}
	
	
	TEST(Move, Equal)
	{
		Move x(905);
		Move y(905);
		Move z(4738);
		ASSERT_TRUE( x == x );
		ASSERT_TRUE( y == y );
		ASSERT_TRUE( z == z );
		ASSERT_TRUE( x == y );
		ASSERT_TRUE( y == x );
		ASSERT_FALSE( x == z );
		ASSERT_FALSE( z == x );
		ASSERT_FALSE( y == z );
		ASSERT_FALSE( z == y );
		
	}
	
	TEST(Move, NotEqual)
	{
		Move x(905);
		Move y(905);
		Move z(4738);
		ASSERT_FALSE( x != x );
		ASSERT_FALSE( y != y );
		ASSERT_FALSE( z != z );
		ASSERT_FALSE( x != y );
		ASSERT_FALSE( y != x );
		ASSERT_TRUE( x != z );
		ASSERT_TRUE( z != x );
		ASSERT_TRUE( y != z );
		ASSERT_TRUE( z != y );
		
	}
	
	TEST(Move, assign)
	{
		Move x(4533);
		Move y = x;
		ASSERT_EQ( 4533, y.getPacked());
	}
	
	TEST(Move, assign2)
	{
		Move x(tSquare::A2, tSquare::D6);
		Move y = x;
		ASSERT_EQ( tSquare::A2, y.getFrom());
		ASSERT_EQ( tSquare::D6, y.getTo());
		ASSERT_FALSE(y.isPromotionMove());
		ASSERT_FALSE(y.isCastleMove());
		ASSERT_FALSE(y.isEnPassantMove());
	}
	
	TEST(Move, assign3)
	{
		Move x(4533);
		x = 12;
		ASSERT_EQ( 12, x.getPacked());
	}
	
	TEST(Move, set1)
	{
		Move x(tSquare::A8, tSquare::B2);
		Move y;
		
		y.setFrom(tSquare::A8);
		y.setTo(tSquare::B2);
		y.setFlag(Move::fnone);
		y.setPromotion(Move::promQueen);
		
		ASSERT_EQ( x, y );
		
	}
	
	TEST(Move, set2)
	{
		Move x(tSquare::E1, tSquare::G1, Move::fcastle);
		Move y;
		
		y.setFrom(tSquare::E1);
		y.setTo(tSquare::G1);
		y.setFlag(Move::fcastle);
		y.setPromotion(Move::promQueen);
		
		ASSERT_EQ( x, y );
		
	}
	
	TEST(Move, set3)
	{
		Move x(tSquare::E1, tSquare::A8, Move::fpromotion, Move::promBishop);
		Move y;
		
		y.setFrom(tSquare::E1);
		y.setTo(tSquare::A8);
		y.setFlag(Move::fpromotion);
		y.setPromotion(Move::promBishop);
		
		ASSERT_EQ( x, y );
		
	}
	
	TEST(Move, clearFlag)
	{
		Move x(tSquare::E1, tSquare::A8, Move::fnone, Move::promBishop);
		Move y;
		
		y.setFrom(tSquare::E1);
		y.setTo(tSquare::A8);
		y.setFlag(Move::fpromotion);
		y.setPromotion(Move::promBishop);
		
		y.clearFlag();
		
		ASSERT_EQ( x, y );
		
	}

	TEST(Move, bool)
	{
		Move x(tSquare::E1, tSquare::A8, Move::fnone, Move::promBishop);
		Move y( Move::NOMOVE);

		ASSERT_TRUE( x );
		ASSERT_FALSE( y );

	}

	
	
	TEST(extMove, constructor)
	{
		extMove x;
		ASSERT_EQ( x.getPacked(), x.getPacked() );
	}
	
	TEST(extMove, constructor2)
	{
		extMove x(41233);
		ASSERT_EQ( 41233, x.getPacked() );
	}
	
	TEST(extMove, constructor3)
	{
		{
			extMove x(tSquare::A2, tSquare::D6);
			
			ASSERT_EQ( tSquare::A2, x.getFrom());
			ASSERT_EQ( tSquare::D6, x.getTo());
			ASSERT_FALSE(x.isPromotionMove());
			ASSERT_FALSE(x.isCastleMove());
			ASSERT_FALSE(x.isEnPassantMove());
		}
		
		{
			extMove y(tSquare::D2, tSquare::C1, Move::fenpassant);
			
			ASSERT_EQ( tSquare::D2, y.getFrom());
			ASSERT_EQ( tSquare::C1, y.getTo());
			ASSERT_FALSE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_TRUE(y.isEnPassantMove());
		}
		
		{
			extMove y(tSquare::E1, tSquare::G1, Move::fcastle);
			
			ASSERT_EQ( tSquare::E1, y.getFrom());
			ASSERT_EQ( tSquare::G1, y.getTo());
			ASSERT_FALSE(y.isPromotionMove());
			ASSERT_TRUE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
		}
		
		{
			extMove y(tSquare::E7, tSquare::E8, Move::fpromotion);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promQueen, y.getPromotionType());
		}
		
		{
			extMove y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promQueen);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promQueen, y.getPromotionType());
		}
		
		{
			extMove y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promRook);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promRook, y.getPromotionType());
		}
		
		{
			extMove y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promBishop);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promBishop, y.getPromotionType());
		}
		
		{
			extMove y(tSquare::E7, tSquare::E8, Move::fpromotion, Move::promKnight);
			
			ASSERT_EQ( tSquare::E7, y.getFrom());
			ASSERT_EQ( tSquare::E8, y.getTo());
			ASSERT_TRUE(y.isPromotionMove());
			ASSERT_FALSE(y.isCastleMove());
			ASSERT_FALSE(y.isEnPassantMove());
			ASSERT_EQ( Move::promKnight, y.getPromotionType());
		}
		
		
		
	}
	
	
	TEST(extMove, copyConstructor)
	{
		extMove x(905);
		extMove y(x);
		ASSERT_EQ( y.getPacked(), x.getPacked() );
	}
	
	TEST(extMove, Equal)
	{
		extMove x(905);
		extMove y(905);
		extMove z(4738);
		ASSERT_TRUE( x == x );
		ASSERT_TRUE( y == y );
		ASSERT_TRUE( z == z );
		ASSERT_TRUE( x == y );
		ASSERT_TRUE( y == x );
		ASSERT_FALSE( x == z );
		ASSERT_FALSE( z == x );
		ASSERT_FALSE( y == z );
		ASSERT_FALSE( z == y );
		
	}
	
	TEST(extMove, NotEqual)
	{
		extMove x(905);
		extMove y(905);
		extMove z(4738);
		ASSERT_FALSE( x != x );
		ASSERT_FALSE( y != y );
		ASSERT_FALSE( z != z );
		ASSERT_FALSE( x != y );
		ASSERT_FALSE( y != x );
		ASSERT_TRUE( x != z );
		ASSERT_TRUE( z != x );
		ASSERT_TRUE( y != z );
		ASSERT_TRUE( z != y );
		
	}
	
	TEST(extMove, assign)
	{
		extMove x(4533);
		extMove y = x;
		ASSERT_EQ( 4533, y.getPacked());
	}
	
	TEST(extMove, assign2)
	{
		extMove x(tSquare::A2, tSquare::D6);
		extMove y = x;
		ASSERT_EQ( tSquare::A2, y.getFrom());
		ASSERT_EQ( tSquare::D6, y.getTo());
		ASSERT_FALSE(y.isPromotionMove());
		ASSERT_FALSE(y.isCastleMove());
		ASSERT_FALSE(y.isEnPassantMove());
	}
	
	TEST(extMove, assign3)
	{
		Move x(4533);
		extMove y;
		y = x;
		ASSERT_EQ( 4533, y.getPacked());
	}
	
	TEST(extMove, set1)
	{
		extMove x(tSquare::A8, tSquare::B2);
		extMove y;
		
		y.setFrom(tSquare::A8);
		y.setTo(tSquare::B2);
		y.setFlag(Move::fnone);
		y.setPromotion(Move::promQueen);
		
		ASSERT_EQ( x, y );
		
	}
	
	TEST(extMove, set2)
	{
		extMove x(tSquare::E1, tSquare::G1, Move::fcastle);
		extMove y;
		
		y.setFrom(tSquare::E1);
		y.setTo(tSquare::G1);
		y.setFlag(Move::fcastle);
		y.setPromotion(Move::promQueen);
		
		ASSERT_EQ( x, y );
		
	}
	
	TEST(extMove, set3)
	{
		extMove x(tSquare::E1, tSquare::A8, Move::fpromotion, Move::promBishop);
		extMove y;
		
		y.setFrom(tSquare::E1);
		y.setTo(tSquare::A8);
		y.setFlag(Move::fpromotion);
		y.setPromotion(Move::promBishop);
		
		ASSERT_EQ( x, y );
		
	}
	
	TEST(extMove, clearFlag)
	{
		extMove x(tSquare::E1, tSquare::A8, Move::fnone, Move::promBishop);
		extMove y;
		
		y.setFrom(tSquare::E1);
		y.setTo(tSquare::A8);
		y.setFlag(Move::fpromotion);
		y.setPromotion(Move::promBishop);
		
		y.clearFlag();
		
		ASSERT_EQ( x, y );
		
	}
	
	
	TEST(extMove, extEqual)
	{
		extMove x(tSquare::E1, tSquare::A8);
		x.setScore(320);
		
		extMove y(tSquare::E1, tSquare::A8);
		y.setScore(-200);
		
		ASSERT_TRUE( x == y );

		
	}
	
	TEST(extMove, extEqual2)
	{
		extMove x(tSquare::E1, tSquare::A8);
		x.setScore(320);
		
		extMove y(tSquare::E2, tSquare::A8);
		y.setScore(-200);
		
		ASSERT_FALSE( x == y );

		
	}
	
	TEST(extMove, extEqual3)
	{
		extMove x(tSquare::E1, tSquare::A8);
		x.setScore(320);
		
		Move y(tSquare::E1, tSquare::A8);
		
		ASSERT_TRUE( x == y );

		
	}
	
	TEST(extMove, extEqual4)
	{
		extMove x(tSquare::E1, tSquare::A8);
		x.setScore(320);
		
		Move y(tSquare::E2, tSquare::A8);

		
		ASSERT_FALSE( x == y );

		
	}
	
	
	TEST(extMove, comparison)
	{
		extMove x(tSquare::E1, tSquare::A8);
		extMove y(tSquare::E2, tSquare::A8);
		
		x.setScore(320);
		y.setScore(-200);
		ASSERT_FALSE( x < y );
		
		x.setScore(-400);
		y.setScore(-200);
		ASSERT_TRUE( x < y );
		
		x.setScore(0);
		y.setScore(0);
		ASSERT_FALSE( x < y );
		
		x.setScore(0);
		y.setScore(1);
		ASSERT_TRUE( x < y );

		
	}
	
}
