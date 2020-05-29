#include "IOContextThread.h"
#include <cassert>
#include "Types.h"
#include <iostream>

namespace IOEvent
{
IOContextThread::IOContextThread()
	:started_(false),
	io_context_(nullptr),
	work_(nullptr)
{

}

IOContextThread::~IOContextThread()
{
	std::cout << "IOContextThread::~IOContextThread()" << std::endl;
	if (started_)
	{
		stop();
	}
}

io_context *IOContextThread::start()
{
	assert(!started_);
	started_ = true;
	thread_ = std::move(std::thread(std::bind(&IOContextThread::threadFunc, this)));
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (!io_context_)
		{
			condition_.wait(lock);
		}
	}
	return get_pointer(io_context_);
}

void IOContextThread::stop()
{
	started_ = false;
	if (io_context_ != nullptr)
	{
		work_->reset();
		io_context_->stop();
		thread_.join();
	}
}


void IOContextThread::threadFunc()
{
	{
		std::unique_lock<std::mutex> lock(mutex_);
		io_context_ = std::make_unique<io_context>();
		work_ = std::make_unique<executor_work_guard<io_context::executor_type>>(make_work_guard(*io_context_));
		condition_.notify_one();
	}
	io_context_->run();
}

}
