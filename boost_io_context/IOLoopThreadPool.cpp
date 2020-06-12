#include "IOLoopThreadPool.h"
#include "IOLoopThread.h"
#include "IOLoop.h"
#include <cassert>

namespace IOEvent
{

IOLoopThreadPool::IOLoopThreadPool(IOLoop *loop)
	:baseLoop_(loop),
	started_(false),
	numThreads_(0),
	next_(0)
{

}

IOLoopThreadPool::~IOLoopThreadPool()
{
	for (auto &ios : threads_)
	{
		ios->stop();
	}
}

void IOLoopThreadPool::run()
{
	assert(!started_);
	started_ = true;
	for (uint32_t i = 0; i < numThreads_; ++i)
	{
		IOLoopThread *loopThread = new IOLoopThread();
		threads_.push_back(std::unique_ptr<IOLoopThread>(loopThread));
		auto *iosPtr = loopThread->start();
		assert(iosPtr != nullptr);
		io_contexts_.push_back(iosPtr);
	}
}


IOLoop * IOLoopThreadPool::getNextIOLoop()
{
	assert(started_);
	IOLoop* ios = baseLoop_;
	if (!io_contexts_.empty())
	{
		// round-robin
		ios = io_contexts_[next_];
		++next_;
		if (static_cast<size_t>(next_) >= io_contexts_.size())
		{
			next_ = 0;
		}
	}
	return ios;
}

std::vector<IOLoop*> IOLoopThreadPool::getAllIOContext()
{
	assert(started_);
	if (io_contexts_.empty())
	{
		return std::vector<IOLoop*>(1, baseLoop_);
	}
	else
	{
		return io_contexts_;
	}
}

}
