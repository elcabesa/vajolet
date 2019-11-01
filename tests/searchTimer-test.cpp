#include <thread>

#include "gtest/gtest.h"
#include "searchTimer.h"

TEST(searchTimer, constructor)
{
	SearchTimer st;
	EXPECT_TRUE(st.getElapsedTime() < 2);
	EXPECT_TRUE(st.getClockTime() < 2);
}

TEST(searchTimer, functional)
{
	SearchTimer st;
	std::this_thread::sleep_for (std::chrono::milliseconds(100));
	auto et = st.getElapsedTime();
	auto ct = st.getClockTime();
	EXPECT_TRUE(et >= 90 && et < 150);
	EXPECT_TRUE(ct >= 90 && ct < 150);
	
	st.resetTimers();
	std::this_thread::sleep_for (std::chrono::milliseconds(100));
	et = st.getElapsedTime();
	ct = st.getClockTime();
	EXPECT_TRUE(et >= 90 && et < 150);
	EXPECT_TRUE(ct >= 90 && ct < 150);
	
	st.resetClockTimer();
	std::this_thread::sleep_for (std::chrono::milliseconds(100));
	et = st.getElapsedTime();
	ct = st.getClockTime();
	EXPECT_TRUE(et >= 190 && et < 300);
	EXPECT_TRUE(ct >= 90 && ct < 150);
}

TEST(searchTimer, copyOperator)
{
	SearchTimer st;
	SearchTimer st2;
	st2 = st;
	std::this_thread::sleep_for (std::chrono::milliseconds(100));
	auto et = st2.getElapsedTime();
	auto ct = st2.getClockTime();
	EXPECT_TRUE(et >= 90 && et < 150);
	EXPECT_TRUE(ct >= 90 && ct < 150);
}
