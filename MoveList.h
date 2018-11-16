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
	
	*Copyright (C) 2007 Pradyumna Kannan.
	*
	*This code is provided 'as-is', without any expressed or implied warranty.
	*In no event will the authors be held liable for any damages arising from
	*the use of this code. Permission is granted to anyone to use this
	*code for any purpose, including commercial applications, and to alter
	*it and redistribute it freely, subject to the following restrictions:
	*
	*1. The origin of this code must not be misrepresented; you must not
	*claim that you wrote the original code. If you use this code in a
	*product, an acknowledgment in the product documentation would be
	*appreciated but is not required.
	*
	*2. Altered source versions must be plainly marked as such, and must not be
	*misrepresented as being the original code.
	*
	*3. This notice may not be removed or altered from any source distribution.
*/

#ifndef MOVELIST_H_
#define MOVELIST_H_

#include <array>
#include <algorithm>

#include "move.h"
#include "vajolet.h"

template <std::size_t N> class MoveList: private std::array< extMove, N >
{
	
public:
/*****************************************************************
*	constructors
******************************************************************/

/*****************************************************************
*	methods
******************************************************************/
	void insert( const Move& m );
	void reset(void);
	unsigned int size() const;
	const Move& get( const unsigned int n ) const;
	const typename std::array<extMove,N >::iterator begin();
	const typename std::array<extMove,N >::iterator end() const;
	const typename std::array<extMove,N >::iterator actualPosition();
	const Move& findNextBestMove(void);
	const Move& getNextMove(void);
	void ignoreMove( const Move& m );



/*****************************************************************
*	members
******************************************************************/
private:
	typename std::array<extMove,N >::iterator _moveListEnd = std::array< extMove, N >::begin();
	typename std::array<extMove,N >::iterator _moveListPosition = std::array< extMove, N >::begin();
	
};

template <std::size_t N>
void MoveList<N>::insert( const Move& m )
{
	//assert( _moveListEnd < N );
	*( _moveListEnd++ ) = m;
}

template <std::size_t N>
inline void MoveList<N>::reset()
{
	_moveListPosition = std::array< extMove, N >::begin();
	_moveListEnd = std::array< extMove, N >::begin();
}

template <std::size_t N>
unsigned int MoveList<N>::size() const
{
	//assert( _moveListEnd < N );
	return _moveListEnd - std::array< extMove, N >::cbegin();
}

template <std::size_t N>
const Move& MoveList<N>::get( const unsigned int n ) const
{
	assert( n < N );
	return (*this)[ n ].m;
}

template <std::size_t N>
const typename std::array<extMove,N >::iterator MoveList<N>::begin() 
{
	return std::array< extMove, N >::begin();
}

template <std::size_t N>
const typename std::array<extMove,N >::iterator MoveList<N>::end() const
{
	return _moveListEnd;
}

template <std::size_t N>
const typename std::array<extMove,N >::iterator MoveList<N>::actualPosition() 
{
	return _moveListPosition;
}
template <std::size_t N>
inline const Move& MoveList<N>::findNextBestMove(void)
{
	const auto max = std::max_element( _moveListPosition, _moveListEnd );
	if( max != _moveListEnd )
	{
		std::swap( *max, *_moveListPosition );
		return ( _moveListPosition++ )->m;
	}
	return NOMOVE;
}

template <std::size_t N>
inline const Move& MoveList<N>::getNextMove(void)
{
	if( _moveListPosition != _moveListEnd )
	{
		return ( _moveListPosition++ )->m;
	}
	return NOMOVE;
}

template <std::size_t N>
inline void MoveList<N>::ignoreMove( const Move& m )
{
	const auto i = std::find( _moveListPosition, _moveListEnd, m );
	if( i != _moveListEnd )
	{
		std::swap(*i, *_moveListPosition);
		++_moveListPosition;
	}
}






#endif /* MOVELIST_H_ */
