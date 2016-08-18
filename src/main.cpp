//
//  batchmp3 main
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
#include <vector>
#include <string>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lame.h"
#include "WavReader.hpp"


class Path {
public:
	typedef char value_type;
	typedef std::basic_string<value_type> string_type;
	const static value_type separator = '/';

	Path() {}
	Path(const value_type *path) : path_(path) {
		// Remove if last character is separator
		if (!path_.empty() && path_.back() == separator) {
			path_.pop_back();
		}
	}
	Path(const Path& obj) : path_(obj.path_) {}
	Path(Path &&obj) : path_(std::move(obj.path_)) {}

	const value_type *c_str() const
	{
		return path_.c_str();
	}

	const value_type *GetExt()
	{
		const size_t len = path_.size();
		for (size_t i = 0; i < len; ++i) {
			if (path_[len - i - 1] == '.') {
				return path_.c_str() + len - i;
			} else if (path_[len - i - 1] == separator) {
				break;
			}
		}
		return path_.c_str() + len;	// return emptry string
	}

	void SetExt(const value_type *ext)
	{
		size_t i = GetExt() - path_.c_str();
		path_.erase(path_.begin() + i, path_.end());
		if (i > 0 && path_[i - 1] != '.') {
			path_.append(1, '.');
		}
		path_.append(ext);
	}
	
	void Append(const string_type& str)
	{
		if (!str.empty()) {
			path_.append(1, separator);
			path_.append(str);
		}
	}

private:
	string_type path_;
};

const Path::value_type *GetExt(const Path::value_type *filename, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		if (filename[len - i - 1] == '.') {
			return filename + len - i;
		}
	}
	return filename + len;	// return emptry string
}

template<class CharT>
std::basic_string<CharT> tolower(std::basic_string<CharT> str)
{
	for (auto& c: str) {
		c = std::tolower(c);
	}
	return std::move(str);
}

std::vector<Path> EnumWavFiles(const char * path)
{
	std::vector<Path> files;

	if (path == nullptr) {
		path = ".";
	}
	
	DIR *dir = opendir(path);
	if (!dir) return files;

	struct dirent *entry;
	while ((entry = readdir(dir))) {
		if (strcmp(tolower<char>(GetExt(entry->d_name, entry->d_namlen)).c_str(), "wav") == 0) {
			Path cur(path);
			cur.Append(entry->d_name);
			
			struct stat st;
			if (stat(cur.c_str(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFREG) {
				printf("%s\n", entry->d_name);
				files.push_back(std::move(cur));
			}
		}
	}

	closedir(dir);

	return files;
}

struct lame_deleter {
	void operator()(lame_global_flags *p) {
		lame_close(p);
	}
};

int main(int argc, const char * argv[])
{
	std::vector<Path> files = EnumWavFiles(argc <= 1 ? nullptr : argv[1]);
	for (const auto& file: files) {
		WavReader reader;
		if (reader.Open(std::make_shared<std::ifstream>(file.c_str(), std::ios::in | std::ios::binary))) {
			std::unique_ptr<lame_global_flags, lame_deleter> lame(lame_init());
			
			const auto& fmt = reader.GetFormat();
			lame_set_num_channels(lame.get(), fmt.channels);
			lame_set_mode(lame.get(), fmt.channels == 1 ? MONO : JOINT_STEREO);
			lame_set_in_samplerate(lame.get(), fmt.sample_rate);
			if (lame_init_params(lame.get()) < 0) {
				std::cerr << "Invalid encoder parameter." << std::endl;
				break;
			}

			Path mp3_file = file;
			mp3_file.SetExt("mp3");
			
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
				int res = lame_encode_buffer_interleaved(lame.get(), reinterpret_cast<short *>(buffer.data()), read_samples,
											   mp3buffer.data(), static_cast<int>(mp3buffer.size()));
				if (res > 0) {
					os.write(reinterpret_cast<const char *>(mp3buffer.data()), res);
				}
			}
			
			mp3buffer.resize(7200);
			int res = lame_encode_flush(lame.get(), mp3buffer.data(), static_cast<int>(mp3buffer.size()));
			if (res > 0) {
				os.write(reinterpret_cast<const char *>(mp3buffer.data()), res);
			}
		}
	}

	return 0;
}
