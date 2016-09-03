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

#ifndef Encoder_h
#define Encoder_h

#include <vector>
#include <deque>
#include <mutex>
#include <pthread.h>
#include "filesystem.h"


class EncodeSourceQueue
{
private:
	std::deque<Path> queue_;
	std::mutex mutex_;
	
public:
	void Push(const std::vector<Path>& files);
	bool Pop(Path& val);
};


class Encoder
{
private:
	pthread_t thread_;
	EncodeSourceQueue *source_queue_;

	Encoder(const Encoder&);
	Encoder(Encoder&& obj);

	static void *ThreadProc(void *arg);
	void *Thread();

public:
	Encoder(EncodeSourceQueue *queue);
	~Encoder();
	
	void Join();
};


#endif /* Encoder_h */
