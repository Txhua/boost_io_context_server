#ifndef _IOEVENT_TCP_CONNECTION_H
#define _IOEVENT_TCP_CONNECTION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include "Callbacks.h"
#include "Buffer.h"

namespace IOEvent
{
using namespace boost::asio;
class TcpConnection final
	: public std::enable_shared_from_this<TcpConnection>,
	public boost::noncopyable
{
public:
	TcpConnection(ip::tcp::socket &&socket, const std::string &name);
	~TcpConnection();
	ip::tcp::endpoint localAddr() const { return socket_.local_endpoint(); }
	ip::tcp::endpoint remoteAddr()const { return socket_.remote_endpoint(); }
	void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
	void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
	void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
	void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); }
	void connectEstablished();
	const std::string &name()const { return name_; }
	Buffer *buffer() { return &buffer_; }
private:
	enum { max_body_length = 1024 };
	void read();
	void write();
private:
	const std::string name_;
	ip::tcp::socket socket_;
	Buffer buffer_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	CloseCallback closeCallback_;
	char buf[5];
};






}


#endif // !_IOEVENT_TCP_CONNECTION_H
