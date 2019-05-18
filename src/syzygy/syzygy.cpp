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

#include "tbCommonData.h"
#include "syzygy.h"

Syzygy::Syzygy() {
	TBCommonData::init();
}

void Syzygy::setPath(const std::string s) {
	TBFile::setPaths(s); 
	_t.clear();
	_t.init();
}

size_t Syzygy::getSize() const {
	return _t.size();
}