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
#ifndef SPSA_H_
#define SPSA_H_

class SPSA {
public:
	SPSA();
	void run();

private:
	static constexpr double alpha = 0.602;
	static constexpr double gamma = 0.101;
	static constexpr int N = 50000;
	static constexpr double A = 5000;
};

#endif