#ifndef _IOEVENT_IO_CONTEXT_THREAD_H
#define _IOEVENT_IO_CONTEXT_THREAD_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "Thread.h"

namespace IOEvent
{
using namespace boost::asio;
class IOContextThread  final : public boost::noncopyable
{
	using io_context_work = executor_work_guard<io_context::executor_type>;
public:
	IOContextThread();
	~IOContextThread();
	io_context *start();
	void stop();
private:
	void threadFunc();
private:
	std::atomic<bool> started_;
	Thread thread_;
	std::unique_ptr<io_context_work> work_;
	std::unique_ptr<io_context> io_context_;
	std::mutex mutex_;
	std::condition_variable condition_;
};

#endif // !_IOEVENT_IO_CONTEXT_THREAD_H

}
