//
//  File system
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

#ifndef filesystem_h
#define filesystem_h

#include <string>
#include <vector>


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

	Path& operator=(const Path& obj)
	{
		path_ = obj.path_;
		return *this;
	}
	
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

std::vector<Path> EnumWavFiles(const char * path);

#endif /* filesystem_h */
