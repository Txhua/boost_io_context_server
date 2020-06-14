//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.


#ifndef _IOEVENT_THREAD_POOL_H
#define _IOEVENT_THREAD_POOL_H

#include <boost/noncopyable.hpp>
#include <future>
#include <deque>
#include "Thread.h"

namespace IOEvent
{
class ThreadPool final : public boost::noncopyable
{
	using Task = std::function<void()>;
public:
	ThreadPool(const std::string &name = std::string());
	~ThreadPool();
	void setmaxQueueSize(int32_t max);
	void start(int32_t threadNum);
	void stop();
	void post(Task task);
	template <typename _FuncType, typename ...Args>
	std::future<typename std::result_of<_FuncType(Args...)>::type> postFuture(_FuncType &&func, Args &&...args);
private:
	Task take();
	void threadFunc();
private:
	int32_t maxQueueSize_;
	const std::string name_;
	std::deque<Task> queue_;
	std::vector<std::unique_ptr<Thread>> threads_;
	std::atomic<bool> runing_;
	std::mutex mutex_;
	std::condition_variable notFull_;
	std::condition_variable notEmpty_;
};

template<typename _FuncType, typename ...Args>
inline std::future<typename std::result_of<_FuncType(Args...)>::type> 
ThreadPool::postFuture(_FuncType &&func, Args && ...args)
{
	using return_type = typename std::result_of<_FuncType(Args...)>::type;
	using task = std::packaged_task<return_type()>;
	auto t = std::make_shared<task>(std::bind(std::forward<_FuncType>(func), std::forward<Args>(args)...));
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if (!runing_) 
		{
			throw std::runtime_error("thread pool has stopped");
		}
		while (maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_)
		{
			// 已满 (只有当 lambad 条件为 false 时调用 wait() 才会阻塞当前线程，并且在收到其他线程的通知后只有当 lambad 为 true 时才会被解除阻塞)
			notFull_.wait(lock, [&]()->bool {return queue_.size() < maxQueueSize_ || !runing_; });
		}
		queue_.emplace([t]() {(*t)(); });
	}
	notEmpty_.notify_one();
	return t->get_future();
}

}


#endif // !_IOEVENT_THREAD_POOL_H

