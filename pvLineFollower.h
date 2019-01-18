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
#ifndef PVLINEFOLLOWER_H_
#define PVLINEFOLLOWER_H_

#include "pvLine.h"

class PVlineFollower
{
public:
	void setPVline( const PVline& p ){ _line = p; }
	
	void restart()
	{
		_followPV = true;
		_maxPvLineRead = -1;
	}
	
	void clear()	
	{
		_line.clear();
	}
	
	inline void getNextMove( const int ply, Move& ttMove)
	{
		if (_followPV)
		{
			if( ply > _maxPvLineRead )
			{
				_maxPvLineRead = std::max( ply, _maxPvLineRead);
		
				// if line is already finished, _stop following PV
				if( ply < (int)_line.size() )
				{
					// overwrite the ttMove
					PVline::iterator it = _line.begin();
					std::advance(it, ply);
					ttMove = *it;
					assert( ttMove == Move::NOMOVE || _pos.isMoveLegal(ttMove) );
					return;
				}
			}
			_disable();
		}
	}
private:

	void _disable()
	{
		_followPV = false;
	}
	PVline _line;
	int _maxPvLineRead = -1;
	bool _followPV = false;
};

#endif