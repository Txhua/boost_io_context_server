#ifndef _IOEVENT_IO_CONTEXT_POOL_H
#define _IOEVENT_IO_CONTEXT_POOL_H

#include <boost/noncopyable.hpp>
#include <boost/asio/io_context.hpp>
#include <vector>
#include <memory>

namespace IOEvent
{
class IOContextThread;
using namespace boost::asio;
class IOContextThreadPool final : public boost::noncopyable
{
public:
	IOContextThreadPool(io_context &ios);
	~IOContextThreadPool();
	void setThreadNum(int numThreads) { numThreads_ = numThreads; }
	io_context * getNextIOContext();
	std::vector<io_context *> getAllIOContext();
	bool started() const { return started_; }
	void run();
private:
	io_context *baseIoContext_;
	std::vector<std::unique_ptr<IOContextThread>> threads_;
	std::vector<io_context *> io_contexts_;
	uint32_t next_;
	std::atomic<bool> started_;
	uint32_t numThreads_;
};

}



#endif // !_IOEVENT_IO_CONTEXT_POOL_H
