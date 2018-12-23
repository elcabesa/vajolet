#include <vector>
#include "gtest/gtest.h"
#include "./../position.h"
#include "./../search.h"
#include "./../searchResult.h"
#include "./../searchLimits.h"
#include "./../searchTimer.h"
#include "./../transposition.h"


enum resType
{
	near,
	bigger,
	smaller,
	equal
};

typedef struct _positions
{
     const std::string Fen;
     unsigned int depth;
     Move bm;
     Move am;
     resType res;
     Score score;

}positions;

static const std::vector<positions> _p =
{
	{"8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1", 30, Move(A1,B1), Move::NOMOVE,bigger, 20000}, // fine 70
	{"6k1/8/7P/7K/8/8/2B5/8 w - - 0 30", 15, Move::NOMOVE, Move::NOMOVE,equal, 0},
	{"8/8/5k2/5P2/5K2/8/8/8 w - - 0 60", 20, Move::NOMOVE, Move::NOMOVE,equal, 0},
	{"4k3/4P3/4K3/8/8/8/8/8 w - - 0 1", 15, Move::NOMOVE, Move::NOMOVE,bigger, 90000},
	{"8/3k4/8/3K4/8/3P4/8/8 w - - 0 1", 15, Move(D3,D4), Move::NOMOVE,bigger, 90000},
	// texel search tests
	{"3k4/8/3K2R1/8/8/8/8/8 w - - 0 1", 2, Move(G6,G8), Move::NOMOVE,equal, mateIn(1)},
	{"8/1P6/k7/2K5/8/8/8/8 w - - 0 1", 4, Move(B7,B8, Move::fpromotion, Move::promQueen),Move::NOMOVE, equal, mateIn(3)},
	{"8/5P1k/5K2/8/8/8/8/8 w - - 0 1", 4, Move(F7,F8, Move::fpromotion, Move::promRook),Move::NOMOVE, equal, mateIn(3)},
	{"3kB3/8/1N1K4/8/8/8/8/8 w - - 0 50",4, Move::NOMOVE,Move::NOMOVE, equal, 0}, //stale mate
	{"8/8/2K5/3QP3/P6P/1q6/8/k7 w - - 31 51", 10, Move::NOMOVE,Move(D5,B3), bigger, 40000 },

	{"2r2rk1/6p1/p3pq1p/1p1b1p2/3P1n2/PP3N2/3N1PPP/1Q2RR1K b - - 0 1", 15, Move(F4,G2),Move::NOMOVE, bigger, 30000 },  //WAC 174
	{"r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PPR/2KR4 w - -", 10, Move(H6,H7),Move::NOMOVE, equal, mateIn(3) }, //WAC 004
	{"7k/1P3R1P/6r1/5K2/8/8/6R1/8 b - - 98 194", 10, Move(G6,G5),Move::NOMOVE, equal, 0 },
	{"7K/6R1/5k2/3q4/8/8/8/8 b - - 0 1",20, Move::NOMOVE,Move::NOMOVE, equal, mateIn(17)},// D5D8 or D5A8 are equal winning

	{"7k/5RR1/8/8/8/8/q3q3/2K5 w - - 0 20",6, Move(G7,H7),Move::NOMOVE, equal, 0},
	//kpn vs k
	{"8/6k1/8/6NP/8/8/6K1/8 w - - 50 1", 10, Move::NOMOVE,Move::NOMOVE, bigger, 50000},
	{"8/6kP/8/6N1/8/8/6K1/8 b - - 50 80", 10, Move(G7,H8),Move::NOMOVE, equal, 0},
	{"8/8/8/8/k7/2n5/p7/K7 w - - 50 80", 10, Move(A1,B2),Move::NOMOVE, equal, 0},
	{"8/1k6/7n/8/8/4p3/4K3/8 w - - 50 50", 3, Move(E2,E3),Move::NOMOVE, equal, 0}, 
	{"8/1k6/7n/8/8/4pK2/8/8 b - - 50 50", 10, Move(H6,F5),Move::NOMOVE, bigger, 50000}, 

};

TEST(search, search) {
	
	Search::initSearchParameters();
	transpositionTable::getInstance().setSize(1);
	Position::initMaterialKeys();
	
	SearchTimer st;
	SearchLimits sl;


	Search src( st, sl, UciOutput::create( UciOutput::mute ) );


	for (auto & p : _p)
	{
		src.getPosition().setupFromFen(p.Fen);
		sl.depth = p.depth;
		auto res = src.startThinking();
		if( p.bm != Move::NOMOVE)
		{
			EXPECT_EQ( res.PV.getMove(0), p.bm);
		}

		if( p.am != Move::NOMOVE)
		{
			EXPECT_NE( res.PV.getMove(0), p.am);
		}
		switch( p.res )
		{
		case bigger:
			EXPECT_GT( res.Res, p.score) << "problem with position: '"<< p.Fen <<"' expected bigger";
			break;
		case smaller:
			EXPECT_LT( res.Res, p.score) << "problem with position: '"<< p.Fen <<"' expected smaller";
			break;
		case near:
			EXPECT_GT( res.Res, p.score - 200) << "problem with position: '"<< p.Fen <<"' expected similar";
			EXPECT_LT( res.Res, p.score + 200) << "problem with position: '"<< p.Fen <<"' expected similar";
			break;
		case equal:
			EXPECT_EQ( res.Res, p.score) << "problem with position: '"<< p.Fen <<"' expected equal";
			break;
		}

	}
}
