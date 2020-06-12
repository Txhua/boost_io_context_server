#ifndef _IOEVENT_IO_LOOP
#define _IOEVENT_IO_LOOP

#include <boost/asio/io_context.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "Timestamp.h"
#include "TimerId.h"
#include "Callbacks.h"

namespace IOEvent
{
using namespace boost::asio;
class TimerQueue;
class IOLoop final 
	: public boost::noncopyable
{
	using FuncCallback = std::function<void()>;
	using executor_work = executor_work_guard<io_context::executor_type>;
public:
	IOLoop();
	~IOLoop();
	bool isInLoopThread()const;
	void assertInLoopThread();
	TimerId runAt(const Timestamp &when, TimerCallback cb);
	TimerId runAfter(double delay, TimerCallback cb);
	TimerId runEvery(double interval, TimerCallback cb);
	void dispatch(FuncCallback cb);
	void post(FuncCallback cb);
	io_context *getContext();
	void cancel(TimerId timerId);
	void loop();
	void quit();
private:
	void abortNotInLoopThread();
	void quitInThisThread();
private:
	io_context ioContext_;
	executor_work work_;
	std::thread::id threadId_;
	std::unique_ptr<TimerQueue> timerQueue_;
	std::atomic<bool> quit_;
};

}




#endif // !_IOEVENT_IO_LOOP
