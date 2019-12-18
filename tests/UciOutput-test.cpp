#include <ios>
#include <sstream>

#include "gtest/gtest.h"
#include "uciOutput.h"
#include "move.h"
#include "tSquare.h"

class UciOutputTest : public ::testing::Test {
protected:
	void SetUp() override {
		
		sbuf = std::cout.rdbuf();
		std::cout.rdbuf(buffer.rdbuf());
	}
	
	void TearDown() override {
		std::cout.rdbuf(sbuf);
	}
	
	std::streambuf *sbuf;
	std::stringstream buffer;
};


TEST_F(UciOutputTest, setDepth)
{
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	UOI->setDepth(3);
	UOI->printDepth();
	
	EXPECT_EQ(buffer.str(), "info depth 3\n");
}

TEST_F(UciOutputTest, printScore)
{
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	UOI->printScore(200);
	EXPECT_EQ(buffer.str(), "info score cp 200\n");
	buffer.str("");
	
	UOI->printScore(-400);
	EXPECT_EQ(buffer.str(), "info score cp -400\n");
	
}

TEST_F(UciOutputTest, printBestMove1) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move bm(tSquare::E1,tSquare::E2);
	Move pm(Move::NOMOVE);
	
	UOI->printBestMove(bm, pm, false);
	
	EXPECT_EQ(buffer.str(), "bestmove e1e2\n");
}

TEST_F(UciOutputTest, printBestMove2) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move bm(tSquare::E1,tSquare::E2);
	Move pm(tSquare::D8,tSquare::C4);
	
	UOI->printBestMove(bm, pm, false);
	
	EXPECT_EQ(buffer.str(), "bestmove e1e2 ponder d8c4\n");
}

TEST_F(UciOutputTest, printBestMove3) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move bm(tSquare::E1,tSquare::E2);
	Move pm(Move::NOMOVE);
	
	UOI->printBestMove(bm, pm, true);
	
	EXPECT_EQ(buffer.str(), "bestmove e1e2\n");
}

TEST_F(UciOutputTest, printBestMove4) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move bm(tSquare::E1,tSquare::E2);
	Move pm(tSquare::D8,tSquare::C4);
	
	UOI->printBestMove(bm, pm, true);
	
	EXPECT_EQ(buffer.str(), "bestmove e1e2 ponder d8c4\n");
}

TEST_F(UciOutputTest, printBestMoveCastle1) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move bm(tSquare::E1,tSquare::H1, Move::fcastle);
	Move pm(tSquare::E8,tSquare::A8, Move::fcastle);
	
	UOI->printBestMove(bm, pm, false);
	
	EXPECT_EQ(buffer.str(), "bestmove e1g1 ponder e8c8\n");
}

TEST_F(UciOutputTest, printBestMoveCastle2) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move bm(tSquare::E1,tSquare::H1, Move::fcastle);
	Move pm(tSquare::E8,tSquare::A8, Move::fcastle);
	
	UOI->printBestMove(bm, pm, true);
	
	EXPECT_EQ(buffer.str(), "bestmove e1h1 ponder e8a8\n");
}

TEST_F(UciOutputTest, printGeneralInfo) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();

	UOI->printGeneralInfo( 32, 966644, 132759, 9576);
	EXPECT_EQ(buffer.str(), "info hashfull 32 tbhits 966644 nodes 132759 time 9576 nps 13863\n");
}

TEST_F(UciOutputTest, printCurrMoveNumber) {
	std::unique_ptr<UciOutput> UOI;
	UOI = UciOutput::create();
	
	Move m(tSquare::D8,tSquare::C4);

	UOI->printCurrMoveNumber( 3, m, 75234, 12, false);
	EXPECT_EQ(buffer.str(), "info currmovenumber 3 currmove d8c4 nodes 75234 time 12\n");
}