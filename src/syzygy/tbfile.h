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

#ifndef TBFILE_H
#define TBFILE_H

#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifndef _WIN32
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/mman.h>
	#include <sys/stat.h>
#else
	#include <windows.h>
#endif

// class TBFile memory maps/unmaps the single .rtbw and .rtbz files. Files are
// memory mapped for best performance. Files are mapped at first access: at init
// time only existence of the file is checked.
class TBFile {
private:
	uint8_t* _baseAddress;
#ifndef _WIN32
	uint64_t _mapping;
#else
	HANDLE _mapping;
#endif
	uint64_t _size;
	static std::string _paths;
	
	static std::string _getFileName(const std::string& f);
	
	void _unmap();

public:
	static void setPaths(std::string path);
	static bool exist(const std::string& f);
	
	uint8_t& operator[](std::size_t idx);
	const uint8_t& operator[](std::size_t idx) const;
	bool isValid() const;
	uint64_t size() const { return _size; };
	
	TBFile ();
	TBFile (const std::string& f);
	~TBFile();

	TBFile(const TBFile& other) = delete; // copy constructor
	TBFile(TBFile&& other) noexcept = delete; // move constructor
	TBFile& operator=(const TBFile& other) = delete; // copy assignment
	TBFile& operator=(TBFile&& other) noexcept;
	
};

#endif