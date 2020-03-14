#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "command.h"

class commandTest : public ::testing::Test {
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

void insertQuit(std::stringstream& str) {
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	str<<std::string("stop\nquit\n");
	
}

TEST_F(commandTest, uciTest) {
	std::string cmd("uci\nposition fen 2k5/7R/2K5/8/8/8/8/8 w - - 0 1 \ngo wtime 1000 btime 1000 winc 200 binc 200 movestogo 2\n");
	std::stringstream str(cmd);
	std::thread th(insertQuit, std::ref(str));
	UciManager::getInstance().uciLoop(str);
	th.join();
	
	std::size_t found = buffer.str().find("bestmove ");
	
	ASSERT_NE(found, std::string::npos);
	
	EXPECT_EQ(buffer.str()/*.substr(found + 9, 4)*/, "h7h8");
	
}

TEST_F(commandTest, PerftTest) {
	std::string cmd("uci\nposition startpos moves e2e4 e7e5 \nperft 3\n");
	std::stringstream str(cmd);
	std::thread th(insertQuit, std::ref(str));
	UciManager::getInstance().uciLoop(str);
	th.join();
	
	std::size_t found = buffer.str().find("leaf nodes: ");
	
	ASSERT_NE(found, std::string::npos);
	
	EXPECT_EQ(buffer.str().substr(found + 12, 5), "24825");
	
}

TEST_F(commandTest, uciTestGoInfinite) {
	std::string cmd("uci\nposition fen 2k5/7R/2K5/8/8/8/8/8 w - - 0 1 \ngo infinite\n");
	std::stringstream str(cmd);
	std::thread th(insertQuit, std::ref(str));
	UciManager::getInstance().uciLoop(str);
	th.join();
	
	std::size_t found = buffer.str().find("bestmove ");
	
	ASSERT_NE(found, std::string::npos);
	
	EXPECT_EQ(buffer.str().substr(found + 9, 4), "h7h8");
	
}

TEST_F(commandTest, ucisetoption) {
	std::string cmd("setoption name Hash value 2\nquit\n");
	std::stringstream str(cmd);
	UciManager::getInstance().uciLoop(str);
	
	std::size_t found = buffer.str().find("info string Hash set to 2");
	
	ASSERT_NE(found, std::string::npos);	
}