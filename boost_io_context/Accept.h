#ifndef _IOEVENT_ACCEPT_H
#define _IOEVENT_ACCEPT_H
#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>

namespace IOEvent
{
using namespace boost::asio;
class IOContextThreadPool;
class Acceptor final : public boost::noncopyable
{
	using NewConnectCallback = std::function<void(ip::tcp::socket &&sock)>;
	using Endpoint = ip::tcp::endpoint;
public:
	explicit Acceptor(io_context &ios, const Endpoint &endpoint);
	~Acceptor();
	void setNewConnectCallback(NewConnectCallback cb);
	void setThreadNum(int numThreads);
	void accept();
	void start();
private:
	ip::tcp::acceptor accept_;
	boost::asio::ip::tcp::socket socket_;
	std::unique_ptr<IOContextThreadPool> ioContextPool_;
	NewConnectCallback newConnectCallback_;
};

}





#endif // !_IOEVENT_ACCEPT_H
