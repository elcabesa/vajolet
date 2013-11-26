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
#ifndef TRANSPOSITION_H_
#define TRANSPOSITION_H_

#include "vajolet.h"
#include <cassert>


struct ttEntry{
	unsigned int key; 			/*! 32 bit for the upper part of the key*/
	Score	value;				/*! 32 bit for the value*/
	Score	staticValue;		/*! 32 bit for the static evalutation (eval())*/
	unsigned short	packedMove;	/*!	16 bit for the move*/
	signed short int depth;		/*! 16 bit for depth*/
	unsigned char searchId;		/*! 8 bit for the generation id*/
	unsigned char type;			/*! 8 bit for the type of the entry*/
								/*  144 bits total =18 bytes*/


};



#endif /* TRANSPOSITION_H_ */
