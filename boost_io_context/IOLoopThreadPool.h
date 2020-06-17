//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_IO_CONTEXT_POOL_H
#define _IOEVENT_IO_CONTEXT_POOL_H

#include <boost/noncopyable.hpp>
#include <boost/asio/io_context.hpp>
#include <vector>
#include <memory>

namespace IOEvent
{

class IOLoopThread;
class IOLoop;
class IOLoopThreadPool final : public boost::noncopyable
{
public:
	IOLoopThreadPool(IOLoop *loop);
	~IOLoopThreadPool();
	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	IOLoop *getNextIOLoop();
	std::vector<IOLoop *> getAllIOContext();
	bool started() const { return started_; }
	void run();
private:
	IOLoop *baseLoop_;
	std::vector<std::unique_ptr<IOLoopThread>> threads_;
	std::vector<IOLoop *> io_contexts_;
	uint32_t next_;
	std::atomic<bool> started_;
	uint32_t numThreads_;
};

}



#endif // !_IOEVENT_IO_CONTEXT_POOL_H
