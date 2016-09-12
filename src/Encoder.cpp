//
//  Encoder
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
#include <fstream>
#include "Encoder.h"
#include "WavReader.h"
#include "util.h"
#include "lame.h"


void EncodeSourceQueue::Push(const std::vector<Path>& files)
{
	std::lock_guard<std::mutex> lock(mutex_);
	queue_.insert(queue_.begin(), files.begin(), files.end());
}

bool EncodeSourceQueue::Pop(Path& val)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (!queue_.empty()) {
		val = queue_.front();
		queue_.pop_front();
		return true;
	}
	return false;
}


Encoder::Encoder(EncodeSourceQueue *queue) : source_queue_(queue)
{
	pthread_create(&thread_, nullptr, ThreadProc, (void *)this);
}

Encoder::~Encoder()
{
	Join();
}

void Encoder::Join()
{
	pthread_join(thread_, nullptr);
}

void *Encoder::ThreadProc(void *arg)
{
	return reinterpret_cast<Encoder *>(arg)->Thread();
}

struct lame_deleter {
	void operator()(lame_global_flags *p) {
		lame_close(p);
	}
};

void *Encoder::Thread()
{
	std::unique_ptr<lame_global_flags, lame_deleter> lame(lame_init());

	Path source_file;
	while (source_queue_->Pop(source_file)) {
		WavReader reader;
		if (reader.Open(std::make_shared<std::ifstream>(source_file.c_str(), std::ios::in | std::ios::binary))) {
			Trace(stdout, _T("Start %s\n"), source_file.c_str());

			std::unique_ptr<lame_global_flags, lame_deleter> lame(lame_init());
			
			const auto& fmt = reader.GetFormat();
			lame_set_num_channels(lame.get(), fmt.channels);
			lame_set_mode(lame.get(), fmt.channels == 1 ? MONO : JOINT_STEREO);
			lame_set_in_samplerate(lame.get(), fmt.sample_rate);
			if (lame_init_params(lame.get()) < 0) {
				Trace(stderr, _T("Invalid encoder parameter.\n"));
				break;
			}
			
			Path mp3_file = source_file;
			mp3_file.SetExt(_T("mp3"));
			
			std::ofstream os(mp3_file.c_str(), std::ios::out | std::ios::binary);
			
			std::vector<unsigned char> buffer(512 * 1024);
			std::vector<unsigned char> mp3buffer;
			uint32_t num_of_samples = reader.GetNumOfSamples();
			for (uint32_t sample_pos = 0; sample_pos < num_of_samples;) {
				int read_samples = reader.ReadSamples(sample_pos, buffer.data(), static_cast<uint32_t>(buffer.size()));
				if (read_samples <= 0) {
					break;
				}
				sample_pos += read_samples;
				
				mp3buffer.resize(read_samples + read_samples / 4 + 7200);
				int res = 0;
				if (reader.GetFormat().format_tag == WavReader::FormatTag_IEEE_FLOAT) {
					res = lame_encode_buffer_interleaved_ieee_float(lame.get(), reinterpret_cast<float *>(buffer.data()), read_samples,
																	mp3buffer.data(), static_cast<int>(mp3buffer.size()));
				} else {
					res = lame_encode_buffer_interleaved(lame.get(), reinterpret_cast<short *>(buffer.data()), read_samples,
														 mp3buffer.data(), static_cast<int>(mp3buffer.size()));
				}
				if (res > 0) {
					os.write(reinterpret_cast<const char *>(mp3buffer.data()), res);
				}
			}
			
			mp3buffer.resize(7200);
			int res = lame_encode_flush(lame.get(), mp3buffer.data(), static_cast<int>(mp3buffer.size()));
			if (res > 0) {
				os.write(reinterpret_cast<const char *>(mp3buffer.data()), res);
			}
			Trace(stdout, _T("Finish %s\n"), source_file.c_str());
		}
	}
	return nullptr;
}
