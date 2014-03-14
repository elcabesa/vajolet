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

#ifndef TUNE_H_
#define TUNE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "io.h"
#include "vajolet.h"

class Tuner{
	std::string epdFile;

	typedef struct sParameterStruct{
		simdScore * parameter;
		//unsigned int parameter;
		unsigned int index;
		std::string name;
		signed int delta;
		sParameterStruct(std::string n,simdScore * p,unsigned int in,signed int d){
			parameter=p;
			index=in;
			name=n;
			delta=d;
		};
	}parameterStruct;

	void showValues(const std::vector<parameterStruct>& parameters);

public:
	//double scaling=18000.0;
	double scaling=13000.0;
	Tuner():epdFile("out2.epd"){}
	double parseEpd(bool save);
	void drawSigmoid(void);
	void createEpd(void);
	void tuneParameters(void);
	void drawAverageEvolution(void);
};



#endif /* TUNE_H_ */
