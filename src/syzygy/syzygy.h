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

#ifndef SYZYGY_H
#define SYZYGY_H

#include <string>

#include "tbfile.h"
#include "tbtables.h"

class Syzygy{
public:
	static Syzygy& getInstance(){
		static Syzygy instance;
		return instance;
	}
  
  	void setPath(const std::string s);
	size_t getSize() const;
private:
	Syzygy()= default;
	~Syzygy()= default;
	Syzygy(const Syzygy&)= delete;
	Syzygy& operator=(const Syzygy&)= delete;

	TBTables _t;
  
};

#endif