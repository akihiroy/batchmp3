//
//  Utility
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

#include <stdio.h>
#include <stdarg.h>
#include <mutex>
#include "util.h"

std::mutex g_TraceMutex;

void Trace(FILE *stream, const char *format, ...)
{
	std::lock_guard<std::mutex> lock(g_TraceMutex);
	va_list args;
	va_start(args, format);
	vfprintf(stream, format, args);
	va_end(args);
}
