//
//  File system for windows
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

#include <windows.h>
#include "filesystem.h"

std::vector<Path> EnumWavFiles(const Path::value_type* path)
{
	std::vector<Path> files;
	
	if (path == nullptr) {
		path = L".";
	} else {
		if (wcscmp(tolower<wchar_t>(GetExt(path, wcslen(path))).c_str(), L"wav") == 0) {
			DWORD attr = GetFileAttributes(path);
			if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
				// path is wav file
				files.emplace_back(path);
				return files;
			}
		}
	}

	std::wstring strFindPath = path;
	strFindPath.append(L"\\*.wav");

	WIN32_FIND_DATA wfd;
	HANDLE hFind = ::FindFirstFile(strFindPath.c_str(), &wfd);
	if (hFind == INVALID_HANDLE_VALUE) {
		return files;
	}

	do {
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		} else {
			Path cur(path);
			cur.Append(wfd.cFileName);
			files.push_back(std::move(cur));
		}
	} while (::FindNextFile(hFind, &wfd));

	::FindClose(hFind);
	
	return files;
}
