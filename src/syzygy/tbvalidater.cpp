/*
	This file is part of Vajolet.
	
	Copyright (c) 2013 Ronald de Man
	Copyright (c) 2015 basil00
	Copyright (C) 2016-2019 Marco Costalba, Lucas Braesch
	Modifications Copyright (c) 2016-2019 by Jon Dart
	Modifications Copyright (c) 2019 by Marco Belli

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

#include "tbfile.h"
#include "tbvalidater.h"

bool TBValidater::validate(const TBFile& tb, const TBType type, const std::string fname) {
	constexpr uint8_t Magics[][4] = 
		{ 
			{ 0xD7, 0x66, 0x0C, 0xA5 },
			{ 0x71, 0xE8, 0x23, 0x5D }
		};
	
	if (tb.size() % 64 != 16)
	{
		std::cerr << "Corrupt tablebase file " << fname << std::endl;
		exit(EXIT_FAILURE);
	}
	
	
	for (int i = 0; i< 4; ++i) {
		if (Magics[type == TBType::WDL][i] != tb[i]) {
			std::cerr << "Corrupted table in file " << fname << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	
	return true;	
}	


