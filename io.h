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

#ifndef IO_H_
#define IO_H_

#include <iostream>

// todo use <syncstream> as soon as implemented by compiler
//--------------------------------------------------------------------
//	enum
//--------------------------------------------------------------------

#define sync_cout std::cout << io_lock				/*!< synchronized cout start*/
#define sync_endl std::endl << io_unlock			/*!< synchronized cout end*/
#define sync_noNewLineEndl io_unlock				/*!< synchronized cout end*/

//--------------------------------------------------------------------
//	enum
//--------------------------------------------------------------------
enum SyncCout { io_lock, io_unlock };


//--------------------------------------------------------------------
//	function prototype
//--------------------------------------------------------------------

std::ostream& operator<<(std::ostream&, SyncCout);

#endif /* IO_H_ */
