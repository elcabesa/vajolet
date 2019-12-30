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

#ifndef COMMAND_H_
#define COMMAND_H_

#include <istream>
#include <memory>

#include "bitBoardIndex.h"

//--------------------------------------------------------------------
//	forward declaration
//--------------------------------------------------------------------
class Move;
class Position;


//--------------------------------------------------------------------
//	function prototype
//--------------------------------------------------------------------
class UciManager
{
private:
	explicit UciManager();
	~UciManager();
	UciManager(const UciManager&) = delete;
	UciManager& operator=(const UciManager&) = delete;
	UciManager(const UciManager&&) = delete;
	UciManager& operator=(const UciManager&&) = delete;
	
	class impl;
	std::unique_ptr<impl> pimpl;
public:

	static UciManager& getInstance()
	{
		static UciManager instance; // Guaranteed to be destroyed.
		// Instantiated on first use.
		return instance;
	}
	void uciLoop(std::istream& is);
};

#endif /* COMMAND_H_ */
