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
#ifndef SEARCH_H_
#define SEARCH_H_

#include <memory>

#include "command.h"
#include "pvLine.h"
#include "score.h"
#include "uciOutput.h"

class Position;
class SearchTimer;
class SearchLimits;
class SearchResult;
class SearchParameters;
class timeManagement;
class transpositionTable;

class Search
{
public:
	//--------------------------------------------------------
	// public static methods
	//--------------------------------------------------------

	//--------------------------------------------------------
	// public methods
	//--------------------------------------------------------
	explicit Search(SearchTimer& st, SearchLimits& sl, transpositionTable& tt, std::unique_ptr<UciOutput> UOI = UciOutput::create());
	~Search();

	Search(const Search& other) = delete;
	Search& operator=(const Search& other) = delete;
	Search(Search&&) =delete;
	Search& operator=(Search&&) = delete;

	void stopSearch();

	unsigned long long getVisitedNodes() const;
	unsigned long long getTbHits() const;
	void showLine();
	SearchResult manageNewSearch(timeManagement & tm);
	Position& getPosition();
	void setUOI( UciOutput::type UOI);
	SearchParameters& getSearchParameters();
	bool setNnue(bool use, std::string path);
	class impl;
private:
	
	std::unique_ptr<impl> pimpl;
};

#endif /* SEARCH_H_ */
