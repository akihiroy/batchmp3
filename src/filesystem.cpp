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

#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "filesystem.h"

std::vector<Path> EnumWavFiles(const Path::value_type * path)
{
	std::vector<Path> files;
	
	if (path == nullptr) {
		path = ".";
	}
	
	DIR *dir = opendir(path);
	if (!dir) return files;
	
	struct dirent *entry;
	while ((entry = readdir(dir))) {
		const size_t len = strlen(entry->d_name);
		if (strcmp(tolower<char>(GetExt(entry->d_name, len)).c_str(), "wav") == 0) {
			Path cur(path);
			cur.Append(entry->d_name);
			
			struct stat st;
			if (stat(cur.c_str(), &st) == 0 && (st.st_mode & S_IFMT) == S_IFREG) {
				files.push_back(std::move(cur));
			}
		}
	}
	
	closedir(dir);
	
	return files;
}
