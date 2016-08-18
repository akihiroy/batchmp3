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

#include <iostream>
#include <assert.h>
#include "WavReader.hpp"

#define FOURCC(fcc) (((uint32_t)(fcc) >> 24 & 0xFF) | (uint32_t)(fcc) >> 8 & 0xFF00 | (uint32_t)(fcc) << 8 & 0xFF0000 | (uint32_t)(fcc) << 24 & 0xFF000000)

struct RiffChunkHeader {
	uint32_t id;
	uint32_t size;
};

bool WavReader::Open(std::shared_ptr<std::istream> is)
{
	try {
		RiffChunkHeader RIFF;
		char RIFFdata[5] = {0};
		is->read(reinterpret_cast<char *>(&RIFF), sizeof(RiffChunkHeader));
		is->read(RIFFdata, 4);

		if (RIFF.id != FOURCC('RIFF') || RIFF.size < 4 || strcmp(RIFFdata, "WAVE") != 0) {
			std::cerr << "This is not WAVE file." << std::endl;
			return false;
		}

		for (uint32_t i = 4; i < RIFF.size; ) {
			RiffChunkHeader chunk_header;
			is->seekg(sizeof(RiffChunkHeader) + i);
			is->read(reinterpret_cast<char *>(&chunk_header), sizeof(RiffChunkHeader));
			i += sizeof(RiffChunkHeader);
			chunks_.insert(std::make_pair(chunk_header.id, Chunk(i, chunk_header.size)));
			//std::cerr << (char)(chunk_header.id & 0xFF) << (char)(chunk_header.id >> 8 & 0xFF) << (char)(chunk_header.id >> 16 & 0xFF) << (char)(chunk_header.id >> 24 & 0xFF) << std::endl;

			if (chunk_header.id == FOURCC('fmt ')) {
				is->read(reinterpret_cast<char *>(&format_), sizeof(Format));
			}
			i += chunk_header.size;
		}
		
		if (chunks_.find(FOURCC('fmt ')) == chunks_.end() || chunks_.find(FOURCC('data')) == chunks_.end()) {
			std::cerr << "This wave file is corrupted." << std::endl;
			return false;
		}

		if (format_.format_tag != 0x01/* Microsoft PCM */) {
			std::cerr << "This codec is not supported." << std::endl;
			return false;
		}

		if (format_.channels != 1 && format_.channels != 2) {
			std::cerr << format_.channels << " channels are not supported." << std::endl;
		}
		
		stream_ = std::move(is);
		return true;
	}
	catch (const std::exception& ex) { std::cerr << ex.what() << std::endl; }
	return false;
}

uint32_t WavReader::GetNumOfSamples() const
{
	auto it = chunks_.find(FOURCC('data'));
	return it == chunks_.end() ? 0 : it->second.size / format_.block_align;
}

int WavReader::ReadSamples(uint32_t sample_pos, void *buf, uint32_t size)
{
	try {
		const auto& data_chunk = chunks_[FOURCC('data')];
		const uint32_t pos = format_.block_align * sample_pos;
		if (pos >= data_chunk.size) {
			return 0;
		}
		stream_->seekg(data_chunk.pos + pos);
		
		size = std::min(size, data_chunk.size - pos) / format_.block_align * format_.block_align;
		stream_->read(static_cast<char *>(buf), size);
		return size / format_.block_align;
	}
	catch (const std::exception& ex) { std::cerr << ex.what() << std::endl; }
	return -1;
}
