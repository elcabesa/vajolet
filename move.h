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

#ifndef MOVE_H_
#define MOVE_H_

struct Move
{
	union
	{
		struct
		{
	unsigned from		:6;
	unsigned to			:6;
	unsigned promotion	:2;
	unsigned flags		:2;
		};
		unsigned short packed;
	};
	enum eflags{
		fnone,
		fpromotion,
		fenpassant,
		fcastle,
	};

	enum epromotion{
		promQueen,
		promRook,
		promBishop,
		promKnight,
	};


};

inline int pawnPush(int color){
	return color? -8:8;
}



#endif /* MOVE_H_ */
