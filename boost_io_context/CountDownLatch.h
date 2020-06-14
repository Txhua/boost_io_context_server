//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_COUNT_DOWN_LATCH_H
#define _IOEVENT_COUNT_DOWN_LATCH_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>

namespace IOEvent
{

class CountDownLatch : public boost::noncopyable
{
public:
	explicit CountDownLatch(int count)
		:count_(count) {}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (count_ > 0)
		{
			condition_.wait(lock);
		}
	}

	void countDown()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		--count_;
		if (count_ == 0)
		{
			condition_.notify_all();
		}
	}

	int getCount()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		return count_;
	}
private:
	std::mutex mutex_;
	std::condition_variable condition_;
	int count_;
};

}

#endif // !_IOEVENT_COUNT_DOWN_LATCH_H
