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

#ifndef NNUE_TYPE_H_
#define NNUE_TYPE_H_ 

#include <cstdint>

//#define CALC_DEBUG_DATA
//#define PRINTSTAT

using outType = uint16_t;
using accumulatorTypeFL = int16_t;
using accumulatorTypeOut = int32_t;
using biasType = int16_t;
using weightType = int16_t;

constexpr int inputSize = 768;
constexpr int accumulatorSize = 64;
constexpr int outSize = 1;

constexpr int scaleFL = 255; //Q10
constexpr int scaleSL = 64; //Q10
constexpr float evalScale = 40000.0f;

/*
using outType = float;
using accumulatorType = float;
using biasType = float;
using weightType = float;
*/
#endif
