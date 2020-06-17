//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

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

class IOLoop;
class TcpConnection final
	: public std::enable_shared_from_this<TcpConnection>,
	public boost::noncopyable
{
	using Socket = boost::asio::ip::tcp::socket;
	using Endpoint = boost::asio::ip::tcp::endpoint;
public:
	explicit TcpConnection(IOLoop *loop, Socket &&socket, const std::string &name);
	~TcpConnection();
	Endpoint localAddr() const { return socket_.local_endpoint(); }
	Endpoint remoteAddr()const { return socket_.remote_endpoint(); }
	void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
	void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
	void setCloseCallback(CloseCallback cb) { closeCallback_ = std::move(cb); }
	// ! 当有新的连接到来时被调用(只能被调用一次)
	void connectEstablished();
	// ! 被TcpServer移除时调用(只能被调用一次)
	void connectDestroyed();
	bool connected()const { return state_ == kConnected; }
	const std::string &name()const { return name_; }
	Buffer *inputBuffer() { return &inputBuffer_; }
	Buffer *outputBuffer() { return &outputBuffer_; }
	// ! 断开连接
	void shutdown();
	// ! 主动关闭
	void forceClose();
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
	void forceCloseInThisThread();
private:
	IOLoop *loop_;
	const std::string name_;
	StateE state_;
	Socket socket_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	CloseCallback closeCallback_;
};






}


#endif // !_IOEVENT_TCP_CONNECTION_H
