﻿//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_IO_CONTEXT_THREAD_H
#define _IOEVENT_IO_CONTEXT_THREAD_H

#include "Thread.h"
#include <atomic>

namespace IOEvent
{
class IOLoop;
class IOLoopThread  final : public boost::noncopyable
{
public:
	IOLoopThread();
	~IOLoopThread();
	IOLoop *start();
	void stop();
private:
	void threadFunc();
private:
	std::atomic<bool> started_;
	Thread thread_;
	IOLoop *loop_;
	std::mutex mutex_;
	std::condition_variable condition_;
};

#endif // !_IOEVENT_IO_CONTEXT_THREAD_H

}
