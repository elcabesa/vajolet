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

TEST(commandTest, init) {
	std::string cmd("uci\nquit\n");
	std::istringstream str(cmd);
	UciManager::getInstance().uciLoop(str);
}
