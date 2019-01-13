#include <vector>
#include "gtest/gtest.h"
#include "./../timeManagement.h"

TEST(timeManagement, initNewSearch0time)
{
	SearchLimits s;
	s.btime = 0;
	s.wtime = 1;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();
	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );
	
	ASSERT_EQ( tm.stateMachineStep( 1, 0 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	
	tm.notifyIterationHasBeenFinished();
	ASSERT_EQ( tm.stateMachineStep( 1, 0 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );
}

TEST(timeManagement, infiniteSearch)
{	
	SearchLimits s;
	s.infinite = true;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.getResolution(), 100 );

	long long int t = 0;
	unsigned long long n = 0;

	for( int i = 0; i < 100000; ++i)
	{
		t += 100;
		n += 800;

		if( i % 17 == 0 )tm.notifyIterationHasBeenFinished();
		if( i % 23 == 0 )tm.notifyFailOver();
		if( i % 29 == 0 )tm.notifyFailLow();

		ASSERT_EQ( tm.stateMachineStep( t, n ), false );

		ASSERT_EQ( tm.isSearchFinished(), false );
	}

	tm.stop();
	for( int i = 0; i < 10000; ++i)
	{
		t += 100;
		n += 800;

		tm.notifyIterationHasBeenFinished();
		ASSERT_EQ( tm.stateMachineStep( t, n ), true );
		ASSERT_EQ( tm.isSearchFinished(), true );
	}
}

TEST(timeManagement, NodesSearchStop)
{
	SearchLimits s;
	s.nodes = 100000000;
	s.checkInfiniteSearch();

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.getResolution(), 100 );

	long long int t = 0;
	unsigned long long n = 0;

	for( int i = 0; i < 100000; ++i)
	{
		t += 100;
		n += 800;

		if( i % 17 == 0 )tm.notifyIterationHasBeenFinished();
		if( i % 23 == 0 )tm.notifyFailOver();
		if( i % 29 == 0 )tm.notifyFailLow();

		ASSERT_EQ( tm.stateMachineStep( t, n ), false );
		ASSERT_EQ( tm.isSearchFinished(), false );
	}

	tm.stop();

	for( int i = 0; i < 10000; ++i)
	{
		t += 100;
		n += 800;

		tm.notifyIterationHasBeenFinished();
		ASSERT_EQ( tm.stateMachineStep( t, n ), true );
		ASSERT_EQ( tm.isSearchFinished(), true );
	}
}

TEST(timeManagement, NodesSearch)
{

	SearchLimits s;
	s.nodes = 1000000;
	s.checkInfiniteSearch();

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.getResolution(), 100 );

	long long int t = 0;
	unsigned long long n = 0;

	for( int i = 0; i < 100000; ++i)
	{
		t += 100;
		n += 800;

		if( i % 17 == 0 )tm.notifyIterationHasBeenFinished();
		if( i % 23 == 0 )tm.notifyFailOver();
		if( i % 29 == 0 )tm.notifyFailLow();

		ASSERT_EQ( tm.stateMachineStep( t, n ), n >= s.nodes ? true: false );
		ASSERT_EQ( tm.isSearchFinished(), n >= s.nodes ? true: false );
	}
}

TEST(timeManagement, moveTimeSearchStop)
{
	SearchLimits s;
	s.moveTime = 100000000;
	s.checkInfiniteSearch();

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.getResolution(), 100 );

	long long int t = 0;
	unsigned long long n = 0;

	for( int i = 0; i < 100000; ++i)
	{
		t += 100;
		n += 800;

		if( i % 17 == 0 )tm.notifyIterationHasBeenFinished();
		if( i % 23 == 0 )tm.notifyFailOver();
		if( i % 29 == 0 )tm.notifyFailLow();

		ASSERT_EQ( tm.stateMachineStep( t, n ), false );
		ASSERT_EQ( tm.isSearchFinished(), false );
	}

	tm.stop();

	for( int i = 0; i < 10000; ++i)
	{
		t += 100;
		n += 800;

		tm.notifyIterationHasBeenFinished();
		ASSERT_EQ( tm.stateMachineStep( t, n ), true );
		ASSERT_EQ( tm.isSearchFinished(), true );
	}
}

TEST(timeManagement, moveTimeSearch)
{

	SearchLimits s;
	s.moveTime = 1000000;
	s.checkInfiniteSearch();

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.getResolution(), 100 );

	long long int t = 0;
	unsigned long long n = 0;

	for( int i = 0; i < 100000; ++i)
	{
		t += 100;
		n += 800;

		if( i % 17 == 0 )tm.notifyIterationHasBeenFinished();
		if( i % 23 == 0 )tm.notifyFailOver();
		if( i % 29 == 0 )tm.notifyFailLow();

		ASSERT_EQ( tm.stateMachineStep( t, n ), t >= s.moveTime ? true: false );
		ASSERT_EQ( tm.isSearchFinished(), t >= s.moveTime ? true: false );
	}
}

TEST(timeManagement, moveTimeSearch2)
{

	SearchLimits s;
	s.moveTime = 1000;
	s.checkInfiniteSearch();

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );
	ASSERT_EQ( tm.getResolution(), 10 );

	s.moveTime = 100;
	tm.initNewSearch( whiteTurn );
	ASSERT_EQ( tm.getResolution(), 1 );

	s.moveTime = 100000;
	tm.initNewSearch( whiteTurn );
	ASSERT_EQ( tm.getResolution(), 100);

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 1000, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 100000000, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyIterationHasBeenFinished();

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 99999, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 100000, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );

}

TEST(timeManagement, normalSearch)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 1000, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 100000000, 100000000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyIterationHasBeenFinished();

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( allocatedTime - 2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 2 , 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );


}

TEST(timeManagement, normalSearchEarlyStop)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int shortAllocatedTime = 571 * 0.7;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();

	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( shortAllocatedTime + 2, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );

}

TEST(timeManagement, normalSearchStop)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	tm.stop();
	ASSERT_EQ( tm.stateMachineStep( allocatedTime - 2, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );

}

TEST(timeManagement, normalSearchNoExtend)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();
	ASSERT_EQ( tm.stateMachineStep( 5, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyFailOver();

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyIterationHasBeenFinished();



	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 2, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );

}

TEST(timeManagement, normalSearchExtend)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();
	ASSERT_EQ( tm.stateMachineStep( 5, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyFailOver();

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 20, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyIterationHasBeenFinished();
	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 20, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );

}

TEST(timeManagement, normalSearchExtend2)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;
	unsigned int maxAllocatedTime = 5710;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();
	ASSERT_EQ( tm.stateMachineStep( 5, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyFailOver();

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 20, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( maxAllocatedTime - 20, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( maxAllocatedTime + 20, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );

}

TEST(timeManagement, normalSearchExtendStop)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;
	unsigned int maxAllocatedTime = 5710;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();
	ASSERT_EQ( tm.stateMachineStep( 5, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.notifyFailOver();

	ASSERT_EQ( tm.stateMachineStep( allocatedTime -2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 20, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.stop();

	ASSERT_EQ( tm.stateMachineStep( maxAllocatedTime - 20, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );


}


TEST(timeManagement, PonderSearch)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.ponder = true;
	s.checkInfiniteSearch();

	unsigned int allocatedTime = 571;

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 1000, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 100000000, 100000000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	s.ponder = false;

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( allocatedTime - 2, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( allocatedTime + 2 , 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );


}

TEST(timeManagement, PonderSearchStop)
{

	SearchLimits s;
	s.btime = 10000;
	s.wtime = 20000;
	s.binc = 0;
	s.winc = 0;
	s.ponder = true;
	s.checkInfiniteSearch();

	timeManagement tm( s );

	tm.initNewSearch( whiteTurn );

	tm.notifyIterationHasBeenFinished();

	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 1000, 10000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );
	ASSERT_EQ( tm.stateMachineStep( 100000000, 100000000 ), false);
	ASSERT_EQ( tm.isSearchFinished(), false );

	tm.stop();


	ASSERT_EQ( tm.stateMachineStep( 100, 10000 ), true);
	ASSERT_EQ( tm.isSearchFinished(), true );


}
