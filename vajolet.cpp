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

#include "vajolet.h"
#include "command.h"
#include "io.h"
#include "bitops.h"
#include "data.h"
#include "hashKeys.h"
#include "position.h"
#include "movegen.h"
#include "transposition.h"
#include "search.h"
#include "eval.h"
#include "syzygy/tbprobe.h"
#include "benchmark.h"


/*!	\brief	print the startup information
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
static void printStartInfo(void)
{
	sync_cout << "id name " << PROGRAM_NAME << " " << VERSION PRE_RELEASE << sync_endl;
}

/*!	\brief	main function
	\author Marco Belli
	\version 1.0
	\date 21/10/2013
*/
int main(int argc, char* argv[])
{
	//----------------------------------
	//	init global data
	//----------------------------------
	std::cout.rdbuf()->pubsetbuf( nullptr, 0 );
	std::cin.rdbuf()->pubsetbuf( nullptr, 0 );
	
	printStartInfo();
	
	initData();
	HashKeys::init();
	Position::initScoreValues();
	Position::initCastleRightsMask();
	Movegen::initMovegenConstant();

	Search::initLMRreduction();
	transpositionTable::getInstance().setSize(1);
	Position::initMaterialKeys();
	tb_init(Search::SyzygyPath.c_str());

	if (argc > 1)
	{
		std::string command = argv[1];
		if( command == "bench" )
		{
			benchmark();
			return 0;
		}
	}
	//----------------------------------
	//	main loop
	//----------------------------------
	
	uciLoop();
	return 0;
}
