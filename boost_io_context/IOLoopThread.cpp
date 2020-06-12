#include "IOLoopThread.h"
#include <cassert>
#include "Types.h"
#include "IOLoop.h"

namespace IOEvent
{
IOLoopThread::IOLoopThread()
	:started_(false),
	loop_(nullptr),
	thread_(std::bind(&IOLoopThread::threadFunc, this))
{

}

IOLoopThread::~IOLoopThread()
{
	if (started_)
	{
		stop();
	}
}

IOLoop *IOLoopThread::start()
{
	assert(!started_);
	assert(!thread_.started());
	started_ = true;
	thread_.start();
	IOLoop *loop = nullptr;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		while (!loop_)
		{
			condition_.wait(lock);
		}
		loop = loop_;
	}
	return loop;
}

void IOLoopThread::stop()
{
	started_ = false;
	if (loop_ != nullptr)
	{
		loop_->quit();
		thread_.join();
	}
}


void IOLoopThread::threadFunc()
{
	IOLoop loop;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		loop_ = &loop;
		condition_.notify_one();
	}
	loop.loop();
	std::unique_lock<std::mutex> lock(mutex_);
	loop_ = nullptr;
}

}
