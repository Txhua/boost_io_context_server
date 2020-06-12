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
class IOLoop;
class TcpConnection final
	: public std::enable_shared_from_this<TcpConnection>,
	public boost::noncopyable
{
public:
	explicit TcpConnection(IOLoop *loop, ip::tcp::socket &&socket, const std::string &name);
	~TcpConnection();
	ip::tcp::endpoint localAddr() const { return socket_.local_endpoint(); }
	ip::tcp::endpoint remoteAddr()const { return socket_.remote_endpoint(); }
	void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
	void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
	void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
	void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); }
	// 当有新的连接到来时被调用(只能被调用一次)
	void connectEstablished();
	// 被TcpServer移除时调用(只能被调用一次)
	void connectDestroyed(); 
	bool connected()const { return state_ == kConnected; }
	const std::string &name()const { return name_; }
	Buffer *inputBuffer() { return &inputBuffer_; }
	Buffer *outputBuffer() { return &outputBuffer_; }
	// 断开连接
	void shutdown();
	void send(Buffer *buf);
	void send(std::string &&message);
	IOLoop *getLoop() { return loop_; }
private:
	enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
	void sendInThisThread(const StringPiece &str);
	void readHeader();
	void readBody(size_t len);
	void write();
	void shutdownInThisThread();
	void setState(StateE s) { state_ = s; }
	void handleClose();
private:
	IOLoop *loop_;
	const std::string name_;
	StateE state_;
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
