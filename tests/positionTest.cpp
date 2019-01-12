#include <vector>
#include "gtest/gtest.h"

#include "./../command.h"
#include "./../data.h"
#include "./../io.h"
#include "./../tSquare.h"
#include "./../move.h"
#include "./../position.h"

//#include "./../movePicker.h"

typedef struct _positions
{
     const std::string Fen;
     const std::vector<unsigned long long> PerftValue;
}positions;

static const std::vector<positions> perftPos ={
	{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", {20ull, 400ull, 8902ull, 197281ull, 4865609ull, 119060324ull}},
	{"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", {48ull, 2039ull, 97862ull, 4085603ull, 193690690ull, 8031647685ull}},
	{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", {14ull, 191ull, 2812ull, 43238ull, 674624ull, 11030083ull, 178633661ull}},
	{"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", {6ull, 264ull, 9467ull, 422333ull, 15833292ull, 706045033ull }},
	{"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1", {6ull, 264ull, 9467ull, 422333ull, 15833292ull, 706045033ull }},
	{"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", {44ull, 1486ull, 62379ull, 2103487ull, 89941194ull }},
	{"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", {46ull, 2079ull, 89890ull, 3894594ull, 164075551ull, 6923051137ull }}
	
};


static unsigned long long myperft(Position& p, unsigned int depth)
{
	if (depth == 0) {
		return 1;
	}

	
	unsigned long long tot = 0;
	for ( unsigned int i = 0; i< 65535; ++i)
	{
		Move m(i);
		if( p.isMoveLegal(m) )
		{
			p.doMove(m);
			tot += myperft(p, depth - 1);
			p.undoMove();
		}
	}
	return tot;

}

TEST(PositionTest, isMoveLegalperft) {
	Position pos;
	for (auto & p : perftPos)
	{
		pos.setupFromFen(p.Fen); 
		for( unsigned int i = 0; i < 2 && i < p.PerftValue.size(); i++)
		{
			EXPECT_EQ(myperft(pos, i+1), p.PerftValue[i]);
		}
	}
}

TEST(PositionTest, isMoveLegal){
	Position pos;
	pos.setupFromFen("3r4/5pk1/2R3p1/7p/1bKP1P1P/r1p3P1/1pB5/1R6 b - - 2 46"); 
	
	Move m(B2, A1, Move::fpromotion, Move::promQueen);
	EXPECT_FALSE(pos.isMoveLegal(m));
}

TEST(PositionTest, setupFromFen){
	Position pos;
	pos.setupFromFen("3r4/5pk1/2R3p1/7p/1bKP1P1P/r1p3P1/1pB5/1R6 b - - 2 46"); 

	EXPECT_EQ(squareNone, pos.getActualState().getEpSquare());
	
}

