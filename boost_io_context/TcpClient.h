//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_TCP_CLIENT_H
#define _IOEVENT_TCP_CLIENT_H

#include <string>
#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include "Callbacks.h"
namespace IOEvent
{
using namespace boost::asio;
class Connector;
class IOLoop;
class TcpClient
{
public:
	TcpClient(IOLoop *loop, const ip::tcp::endpoint &serverAddr,const std::string &name);
	~TcpClient();
	void connect();
	void disconnect();
	void stop();
	const std::string &name() const { return name_; }
	bool retry()const { return retry_; }
	bool enableRetry();
	TcpConnectionPtr connection()const;
	IOLoop *getLoop() { return loop_; }
	void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
	void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
private:
	void newConnection(ip::tcp::socket &&socket);
	void removeConnection(const TcpConnectionPtr &conn);
private:
	IOLoop *loop_;
	const std::string name_;
	std::shared_ptr<Connector> connector_;
	MessageCallback messageCallback_;
	ConnectionCallback connectionCallback_;
	mutable std::mutex mutex_;
	int32_t nextConnId_;
	std::atomic<bool> connect_;
	std::atomic<bool> retry_;
	TcpConnectionPtr connection_;
};
}







#endif // !_IOEVENT_TCP_CLIENT_H

