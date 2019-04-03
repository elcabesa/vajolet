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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <fstream>
#include <stack>

#include "command.h"
#include "move.h"


class logger
{
	std::ofstream ofs;
	unsigned long long nodeId = 0;
	std::stack<unsigned long long> st;
public:
	void open(unsigned int depth)
	{
		std::string s("log");
		s+=std::to_string(depth);
		s+=".dot";
		ofs.open (s, std::ofstream::out | std::ofstream::trunc);
		ofs<< "digraph g {"<<std::endl;

		while(!st.empty())
		{
			st.pop();
		}
		nodeId = 0;


	}

	void addNode(Move m)
	{
		if( st.size()!=0)
		{

			unsigned long long startNode = st.top();
			unsigned long long endNode = nodeId;
			ofs <<"\tN"<<std::to_string(startNode)<<" -> N"<<std::to_string(endNode)<<" [label=\""<< UciManager::displayUci(m)<< "\"];"<<std::endl;
		}
		st.push(nodeId);
		nodeId++;

	}
	void closeNode(void)
	{
		st.pop();
	}


	void close(void)
	{
		ofs<< "}"<<std::endl;
		ofs.close();
	}
};



#endif /* LOGGER_H_ */
