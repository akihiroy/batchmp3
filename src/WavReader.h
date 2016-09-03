//
//  Wave file reader
//
//  Copyright (c) 2016 Akihiro Yamasaki. All rights reserved.
//
// This file is part of batchmp3.
//
// batchmp3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.

// batchmp3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with batchmp3.  If not, see <http://www.gnu.org/licenses/>.

#ifndef WavReader_hpp
#define WavReader_hpp

#include <memory>
#include <istream>
#include <unordered_map>


class WavReader
{
public:
	struct Format {
		uint16_t format_tag;
		uint16_t channels;
		uint32_t sample_rate;
		uint32_t avg_bytes_per_sec;
		uint16_t block_align;
		uint16_t bps;				///< bits per sample
	};

	enum FormatTag {
		FormatTag_PCM = 0x01,
		FormatTag_IEEE_FLOAT = 0x03
	};
	
	bool Open(std::shared_ptr<std::istream> is);
	
	const Format& GetFormat() const { return format_; }
	uint32_t GetNumOfSamples() const;

	int ReadSamples(uint32_t sample_pos, void *buf, uint32_t size);
	
private:
	struct Chunk {
		uint32_t pos;
		uint32_t size;
		Chunk() : pos(0), size(0) {}
		Chunk(uint32_t p, uint32_t s) : pos(p), size(s) {}
	};
	std::unordered_map<uint32_t, Chunk> chunks_;
	
	Format format_;
	std::shared_ptr<std::istream> stream_;
};

#endif /* WavReader_hpp */
