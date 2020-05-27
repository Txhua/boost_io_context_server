#include "IOContextThreadPool.h"
#include "IOContextThread.h"
#include <cassert>

namespace IOEvent
{

IOContextThreadPool::IOContextThreadPool(io_context &ios)
	:baseIoContext_(&ios),
	started_(false),
	numThreads_(0),
	next_(0)
{

}

IOContextThreadPool::~IOContextThreadPool()
{
	for (auto &ios : threads_)
	{
		ios->stop();
	}
}

void IOContextThreadPool::run()
{
	assert(!started_);
	started_ = true;
	for (uint32_t i = 0; i < numThreads_; ++i)
	{
		IOContextThread *iosThread = new IOContextThread();
		threads_.push_back(std::unique_ptr<IOContextThread>(iosThread));
		auto *iosPtr = iosThread->start();
		assert(iosPtr != nullptr);
		io_contexts_.push_back(iosPtr);
	}
}


io_context * IOContextThreadPool::getNextIOContext()
{
	assert(started_);
	io_context* ios = baseIoContext_;
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

std::vector<io_context*> IOContextThreadPool::getAllIOContext()
{
	assert(started_);
	if (io_contexts_.empty())
	{
		return std::vector<io_context*>(1, baseIoContext_);
	}
	else
	{
		return io_contexts_;
	}
}

}
