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
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	str<<std::string("quit\n");
	
}

TEST_F(commandTest, uciTest) {
	std::string cmd("uci\nposition fen 2k5/7R/2K5/8/8/8/8/8 w - - 0 1 \ngo wtime 1000\n");
	std::stringstream str(cmd);
	std::thread th(insertQuit, std::ref(str));
	UciManager::getInstance().uciLoop(str);
	th.join();
	
	std::size_t found = buffer.str().find("bestmove ");
	
	ASSERT_NE(found, std::string::npos);
	
	EXPECT_EQ(buffer.str().substr(found + 9, 4), "h7h8");
	
}
