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
	uint8_t* baseAddress;
#ifndef _WIN32
	uint64_t mapping;
#else
	HANDLE mapping;
#endif

	static std::string _paths;
	
	static std::string _getFileName(const std::string& f) {
		std::string _fname;
		std::ifstream _stream;
#ifndef _WIN32
		constexpr char SepChar = ':';
#else
		constexpr char SepChar = ';';
#endif
		std::stringstream ss(_paths);
		std::string path;

		while (std::getline(ss, path, SepChar)) {
			_fname = path + "/" + f;
			_stream.open(_fname);
			if (_stream.is_open()) {
				_stream.close();
				return _fname;
			}
		}
		return "";
	}
	
	void _unmap() {
		if( baseAddress != nullptr) {
#ifndef _WIN32
			munmap(baseAddress, mapping);
#else
			UnmapViewOfFile(baseAddress);
			CloseHandle(mapping);
#endif
		}
	}

public:

	uint8_t& operator[](std::size_t idx) { return baseAddress[idx]; }
    const uint8_t& operator[](std::size_t idx) const { return baseAddress[idx]; }
	static void setPaths(std::string path) { _paths = path; }
	
	static bool exist(const std::string& f) { return _getFileName(f) != ""; }

	// Look for and open the file among the Paths directories where the .rtbw
	// and .rtbz files can be found. Multiple directories are separated by ";"
	// on Windows and by ":" on Unix-based operating systems.
	//
	// Example:
	// C:\tb\wdl345;C:\tb\wdl6;D:\tb\dtz345;D:\tb\dtz6
	
	// todo type can be undestood from file name extension
	TBFile (const std::string& f) {
		auto fname = _getFileName(f);
		if (fname == "") {
			baseAddress = nullptr;
			mapping = 0;
			return;
		}

#ifndef _WIN32
		struct stat statbuf;
		int fd = open(fname.c_str(), O_RDONLY);
		if (fd == -1) {
			baseAddress = nullptr;
			mapping = 0;
			return;
		}
		
		fstat(fd, &statbuf);
		mapping = statbuf.st_size;
		baseAddress = static_cast<uint8_t*>(mmap(nullptr, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0));
		madvise(baseAddress, statbuf.st_size, MADV_RANDOM);
		close(fd);

		if (baseAddress == MAP_FAILED) {
			std::cerr << "Could not mmap() " << fname << std::endl;
			exit(1);
		}
#else
		// Note FILE_FLAG_RANDOM_ACCESS is only a hint to Windows and as such may get ignored.
		HANDLE fd = CreateFile(fname.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
			OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, nullptr);
			
		if (fd == INVALID_HANDLE_VALUE) {
			baseAddress = nullptr;
			mapping = 0;
			return;
		}

		DWORD size_high;
		DWORD size_low = GetFileSize(fd, &size_high);
		HANDLE mmap = CreateFileMapping(fd, nullptr, PAGE_READONLY, size_high, size_low, nullptr);
		CloseHandle(fd);

		if (!mmap) {
			std::cerr << "CreateFileMapping() failed" << std::endl;
			exit(1);
		}
		
		mapping = mmap;
		baseAddress = static_cast<uint8_t*>(MapViewOfFile(mmap, FILE_MAP_READ, 0, 0, 0));

		if (!baseAddress) {
			std::cerr << "MapViewOfFile() failed, name = " << fname
								<< ", error = " << GetLastError() << std::endl;
			exit(1);
		}
#endif
		// todo remove from here
		// todo check size is multiple of 16
		/*constexpr uint8_t Magics[][4] =
			{ { 0xD7, 0x66, 0x0C, 0xA5 },
			{ 0x71, 0xE8, 0x23, 0x5D } };

		if (memcmp(baseAddress, Magics[type == WDL], 4)) {
				std::cerr << "Corrupted table in file " << fname << std::endl;
				unmap();
				baseAddress = nullptr;
				mapping = 0;
				return;
		}*/

		//return data + 4; // Skip Magics's header
	}

	~TBFile() { _unmap(); }
	
};

#endif