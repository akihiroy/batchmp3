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

#include <vector>
#include <list>
#include <thread>

#include "WavReader.h"
#include "filesystem.h"
#include "Encoder.h"
#include "util.h"

#if !defined(_WIN32)
#define _tmain main
#define TCHAR char
#endif


int _tmain(int argc, TCHAR * argv[])
{
	std::vector<Path> files = EnumWavFiles(argc <= 1 ? nullptr : argv[1]);
	if (files.empty()) {
		Trace(stdout, _T("No wav files found.\n"));
		Trace(stdout, _T("usage: batchmp3 [<source_path>]\n"));
		return 1;
	} else if (files.size() == 1) {
		Trace(stdout, _T("1 wav file found.\n"));
	} else {
		Trace(stdout, _T("%d wav files found.\n"), files.size());
	}

	EncodeSourceQueue source_queue;
	source_queue.Push(files);

	std::list<Encoder> encoders;
	unsigned int concurrency = std::thread::hardware_concurrency();
	
	// Start encoding
	for (unsigned int i = 0; i < concurrency; ++i) {
		encoders.emplace_back(&source_queue);
	}
	
	// Wait for finishing
	for (auto& encoder : encoders) {
		encoder.Join();
	}

	return 0;
}
