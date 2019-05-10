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

#include <array>

// Numbers in little endian used by sparseIndex[] to point into blockLength[]
class SparseEntry {
private:
	char _block[4];		// Number of block
	char _offset[2];	// Offset within the block
public:
	uint32_t getBlock() { return __builtin_bswap32(*reinterpret_cast<uint32_t *>(_block)); }
	uint16_t getOffset() { return __builtin_bswap16(*reinterpret_cast<uint16_t *>(_offset)); }
	
	SparseEntry() = delete;
	~SparseEntry() = delete;
	SparseEntry(const SparseEntry& other) = delete;
	SparseEntry(SparseEntry&& other) noexcept = delete;
	SparseEntry& operator=(const SparseEntry& other) = delete;
	SparseEntry& operator=(SparseEntry&& other) noexcept = delete;

};

static_assert(sizeof(SparseEntry) == 6, "SparseEntry must be 6 bytes");