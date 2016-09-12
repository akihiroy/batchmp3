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
#include <locale>
#include "util.h"


class Path {
public:
#if defined(_WIN32)
	typedef wchar_t value_type;
	const static value_type separator = _T('\\');
#else
	typedef char value_type;
	const static value_type separator = '/';
#endif
	typedef std::basic_string<value_type> string_type;

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
			if (path_[len - i - 1] == _T('.')) {
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
		if (i > 0 && path_[i - 1] != _T('.')) {
			path_.append(1, _T('.'));
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


namespace {
	inline const Path::value_type *GetExt(const Path::value_type *filename, size_t len)
	{
		for (size_t i = 0; i < len; ++i) {
			if (filename[len - i - 1] == _T('.')) {
				return filename + len - i;
			}
		}
		return filename + len;	// return emptry string
	}

	template<class CharT>
	std::basic_string<CharT> tolower(std::basic_string<CharT> str)
	{
		std::locale loc;
		for (auto& c : str) {
			c = std::tolower(c, loc);
		}
		return std::move(str);
	}
}

std::vector<Path> EnumWavFiles(const Path::value_type * path);

#endif /* filesystem_h */
