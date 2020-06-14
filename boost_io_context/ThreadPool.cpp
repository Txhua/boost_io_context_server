#include "ThreadPool.h"
#include <assert.h>
#include <glog/logging.h>

namespace IOEvent
{

ThreadPool::ThreadPool(const std::string &name)
	:name_(name),
	maxQueueSize_(0),
	runing_(false)
{
}

ThreadPool::~ThreadPool()
{
	if (runing_)
	{
		stop();
	}
}

void ThreadPool::setmaxQueueSize(int32_t max)
{
	maxQueueSize_ = max;
}

void ThreadPool::start(int32_t threadNum)
{
	assert(threads_.empty());
	runing_ = true;
	int32_t num = threadNum;
	if (num <= 0)
	{
		num = std::thread::hardware_concurrency();
	}
	threads_.reserve(num);
	for (auto i = 0; i < num; ++i)
	{
		threads_.push_back(std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this), name_));
		threads_[i]->start();
	}
}

void ThreadPool::post(Task task)
{
	std::unique_lock<std::mutex> lock(mutex_);
	if (!runing_)
	{
		throw std::runtime_error("thread pool has stopped");
	}
	while ((maxQueueSize_ > 0 && maxQueueSize_ == queue_.size()))
	{
		notFull_.wait(lock, [&]()->bool {return queue_.size() < maxQueueSize_ ||!runing_; });
	}
	queue_.push_back(std::move(task));
	notEmpty_.notify_one();
}


void ThreadPool::stop()
{
	assert(runing_);
	{
		std::unique_lock<std::mutex> lock(mutex_);
		runing_ = false;
		notEmpty_.notify_all();
		notFull_.notify_all();
	}
	for (auto &t : threads_)
	{
		t->joined();
	}
}

ThreadPool::Task ThreadPool::take()
{
	std::unique_lock<std::mutex> lock(mutex_);
	while (queue_.empty() && runing_)
	{
		notEmpty_.wait(lock, [&]()->bool {return queue_.size() > 0; });
	}
	Task task;
	if (!queue_.empty())
	{
		task = std::move(queue_.front());
		queue_.pop_front();
		if (maxQueueSize_ > 0)
		{
			notFull_.notify_one();
		}
	}
	return std::move(task);
}

void ThreadPool::threadFunc()
{
	try
	{
		while (runing_)
		{
			Task task(take());
			task();
		}
	}
	catch (const std::exception &ex)
	{
		LOG(FATAL) << "exception caught in ThreadPool : " << name_.c_str() << " reason: " << ex.what();
	}
}

}
