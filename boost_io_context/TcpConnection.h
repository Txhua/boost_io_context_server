#ifndef _IOEVENT_TCP_CONNECTION_H
#define _IOEVENT_TCP_CONNECTION_H

#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <memory>
#include "Callbacks.h"
#include "Buffer.h"
#include <array>

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
	Buffer *inputBuffer() { return &inputBuffer_; }
	Buffer *outputBuffer() { return &outputBuffer_; }
	void shutdown();
	void send();
	void send(Buffer *buf);
	void sendInThisThread(const StringPiece &str);
	boost::asio::executor getIoService() { return std::move(socket_.get_executor()); }
private:
	void readHeader();
	void readBody();
	void write();
	void shutdownInThisThread();
private:
	const std::string name_;
	ip::tcp::socket socket_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	CloseCallback closeCallback_;
};






}


#endif // !_IOEVENT_TCP_CONNECTION_H
