#include <vector>
#include "gtest/gtest.h"
#include "./../position.h"
#include "./../search.h"
#include "./../transposition.h"


enum resType
{
	draw,
	won,
	lost
};

typedef struct _positions
{
     const std::string Fen;
     unsigned int depth;
     Move bm;
     resType res;

}positions;

static const std::vector<positions> endgames =
{
	{"8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1", 30, Move(A1,B1), won},
	{"6k1/8/7P/7K/8/8/2B5/8 w - - 0 30", 15, Move::NOMOVE, draw},
	{"4k3/4P3/4K3/8/8/8/8/8 b - - 0 1", 15, Move::NOMOVE, lost},
	{"4k3/4P3/4K3/8/8/8/8/8 w - - 0 1", 15, Move::NOMOVE, won},
	{"8/3k4/8/3K4/8/3P4/8/8 w - - 0 1", 15, Move(D3,D4), won},
};

TEST(PerftTest, endgames) {
	
	Search::initSearchParameters();
	transpositionTable::getInstance().setSize(1);
	Position::initMaterialKeys();
	
	SearchTimer st;
	SearchLimits sl;


	Search src( st, sl, UciOutput::create( UciOutput::mute ) );


	for (auto & p : endgames)
	{
		src.pos.setupFromFen(p.Fen);
		sl.depth = p.depth;
		auto res = src.startThinking();
		if( p.bm != Move::NOMOVE)
		{
			EXPECT_EQ( res.PV.getMove(0), p.bm);
		}
		switch( p.res )
		{
		case won:
			EXPECT_GT( res.Res, 30000) << "problem with position: '"<< p.Fen <<"' expected win";
			break;
		case lost:
			EXPECT_LT( res.Res, -30000) << "problem with position: '"<< p.Fen <<"' expected lost";
			break;
		case draw:
			EXPECT_GT( res.Res, -200) << "problem with position: '"<< p.Fen <<"' expected draw";
			EXPECT_LT( res.Res, +200) << "problem with position: '"<< p.Fen <<"' expected draw";
			break;
		}

	}
}
