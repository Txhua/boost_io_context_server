#ifndef _IOEVENT_THREAD_H
#define _IOEVENT_THREAD_H

#include <functional>
#include <thread>
#include <string>
#include "CountDownLatch.h"
#include <boost/noncopyable.hpp>

namespace IOEvent
{

class Thread : public boost::noncopyable
{
public:
	using ThreadFunc = std::function<void()>;
	Thread(ThreadFunc func, const std::string &name = std::string());
	~Thread();
	void start();
	void join();
	std::string name()const { return name_; }
	bool join()const { return join_; }
	bool started()const { return started_; }
	std::thread::id tid()const { return tid_; }
private:
	ThreadFunc func_;
	std::string name_;
	std::thread thread_;
	std::thread::id tid_;
	CountDownLatch latch_;
	bool started_;
	bool join_;
};

}

#endif // !_IOEVENT_THREAD_H