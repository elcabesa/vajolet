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

#ifndef MULTIPVMANAGER_H_
#define MULTIPVMANAGER_H_

#include <algorithm>
#include <vector>

#include "rootMove.h"

class MultiPVManager
{
public:
	void clean(){ _res.clear(); _previousRes.clear();}
	void startNewIteration(){ _previousRes = _res; _res.clear(); _multiPvCounter = 0;}
	void goToNextPV(){ ++_multiPvCounter; }
	void insertMove( const rootMove& m ){ _res.push_back(m); }
	void setLinesToBeSearched( unsigned int l ){ _linesToBeSearched = l;}
	bool thereArePvToBeSearched() const { return _multiPvCounter < _linesToBeSearched; }
	
	unsigned int getPreviousIterationSize() const { return _previousRes.size(); }
	bool getPreviousIterationRootMove(rootMove& rm) const
	{
		if ( _multiPvCounter < getPreviousIterationSize() )
		{			
			rm = _previousRes[_multiPvCounter]; 
			return true;
		}
		return false;
	}
	
	unsigned int getLinesToBeSearched() const { return _linesToBeSearched; }
	unsigned int getPVNumber() const { return _multiPvCounter;}
	
	std::vector<rootMove> get()
	{	
		// Sort the PV lines searched so far and update the GUI				
		std::stable_sort(_res.begin(), _res.end());
		
		std::vector<rootMove> _multiPVprint = _res;
		// always send all the moves toghether in multiPV
		int missingLines = _linesToBeSearched - _multiPVprint.size();
		if( missingLines > 0 )
		{
			// add result from the previous iteration
			// try adding the missing lines (  uciParameters::multiPVLines - _multiPVprint.size() ) , but not more than previousIterationResults.size() 
			int addedLines = 0;
			for( const auto& m: _previousRes )
			{
				if( std::find(_multiPVprint.begin(), _multiPVprint.end(), m) == _multiPVprint.end() )
				{
					_multiPVprint.push_back(m);
					++addedLines;
					if( addedLines >= missingLines )
					{
						break;
					}
				}
			}
		}
		assert(_multiPVprint.size() == _linesToBeSearched);
		
		return _multiPVprint;
	}
	
private:
	std::vector<rootMove> _res;
	std::vector<rootMove> _previousRes;
	unsigned int _linesToBeSearched;
	unsigned int _multiPvCounter;
};


#endif