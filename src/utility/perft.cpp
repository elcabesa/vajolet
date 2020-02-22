/*
	This file is part of Vajolet.
	Copyright (C) 2013-2018 Marco Belli
	
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

#include "vajo_io.h"
#include "movepicker.h"
#include "perft.h"
#include "position.h"
#include "transposition.h"
#include "uciOutput.h"
#include "vajolet.h"


bool Perft::perftUseHash = false;

/*! \brief calculate the perft result
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
unsigned long long Perft::perft(unsigned int depth)
{

#define FAST_PERFT
#ifndef FAST_PERFT
	if (depth == 0) {
		return 1;
	}
#endif
	PerftTranspositionTable tt;
	
	unsigned long long tot;
	if( perftUseHash && tt.retrieve(_pos.getKey(), depth, tot) )
	{
		return tot;
	}
	
#ifdef FAST_PERFT
	
	if(depth==1)
	{
		return _pos.getNumberOfLegalMoves();
	}
#endif

	tot = 0;
	Move m;
	MovePicker mp(_pos);
	while( ( m = mp.getNextMove() ) )
	{
		_pos.doMove(m);
		tot += perft(depth - 1);
		_pos.undoMove();
	}
	
	if( perftUseHash )
	{
		tt.store(_pos.getKey(), depth, tot);
	}
	
	return tot;

}

/*! \brief calculate the divide result
	\author Marco Belli
	\version 1.0
	\date 08/11/2013
*/
unsigned long long Perft::divide(unsigned int depth)
{


	MovePicker mp(_pos);
	unsigned long long tot = 0;
	unsigned int mn=0;
	Move m;
	while ( ( m = mp.getNextMove() ) )
	{
		mn++;
		_pos.doMove(m);
		unsigned long long n= 1;
		if( depth>1)
		{
			n= perft(depth - 1);
		}
		else
		{
			n= 1;
		}
		tot += n;
		_pos.undoMove();
		sync_cout<<mn<<") "<<UciOutput::displayMove(_pos, m)<<": "<<n<<sync_endl;
	}
	return tot;

}