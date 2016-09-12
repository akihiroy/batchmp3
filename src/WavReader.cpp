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
#include <algorithm>
#include <assert.h>
#include "WavReader.h"
#include "util.h"

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
			Trace(stderr, _T("This is not WAVE file.\n"));
			return false;
		}

		for (uint32_t i = 4; i < RIFF.size; ) {
			RiffChunkHeader chunk_header;
			is->seekg(sizeof(RiffChunkHeader) + i);
			is->read(reinterpret_cast<char *>(&chunk_header), sizeof(RiffChunkHeader));
			i += sizeof(RiffChunkHeader);
			chunks_.insert(std::make_pair(chunk_header.id, Chunk(sizeof(RiffChunkHeader) + i, chunk_header.size)));
			//Trace(stderr, "%c%c%c%c\n", (chunk_header.id & 0xFF), (chunk_header.id >> 8 & 0xFF), (chunk_header.id >> 16 & 0xFF), (chunk_header.id >> 24 & 0xFF));

			if (chunk_header.id == FOURCC('fmt ')) {
				is->read(reinterpret_cast<char *>(&format_), sizeof(Format));
			}
			i += chunk_header.size;
		}
		
		if (chunks_.find(FOURCC('fmt ')) == chunks_.end() || chunks_.find(FOURCC('data')) == chunks_.end()) {
			Trace(stderr, _T("This wave file is corrupted.\n"));
			return false;
		}

		if (format_.format_tag != FormatTag_PCM && format_.format_tag != FormatTag_IEEE_FLOAT) {
			Trace(stderr, _T("This codec is not supported.\n"));
			return false;
		}

		if (format_.format_tag == FormatTag_PCM && format_.bps != 16) {
			Trace(stderr, _T("%d bps is not supported.\n"), format_.bps);
		}
		
		if (format_.channels != 1 && format_.channels != 2) {
			Trace(stderr, _T("%d channels are not supported.\n"), format_.channels);
			return false;
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
