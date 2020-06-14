//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_TCP_SERVER_H
#define _IOEVENT_TCP_SERVER_H

#include "Callbacks.h"
#include <boost/asio/ip/tcp.hpp>
#include <atomic>
#include <map>
namespace IOEvent
{
using namespace boost::asio;
class Acceptor;
class IOLoop;
class IOLoopThreadPool;
class TcpServer
{
	using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
public:
	TcpServer(IOLoop *loop, const ip::tcp::endpoint &endpoint);
	~TcpServer();
	void start();
	const std::string &ipPort()const { return ipPort_; }
	void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
	void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
	void setThreadNum(int numThreads);
	IOLoop *getLoop() const { return baseLoop_; }
private:
	void removeConnection(const TcpConnectionPtr &conn);
	void removeConnectionInThisThread(const TcpConnectionPtr &conn);
	void newConnection(ip::tcp::socket &&socket);
private:
	IOLoop *baseLoop_;
	uint32_t nextConnId_;
	const std::string ipPort_;
	std::atomic<bool> started_;
	std::unique_ptr<Acceptor> accept_;
	std::shared_ptr<IOLoopThreadPool> threadPool_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	ConnectionMap connections_;
};

}




#endif // !_IOEVENT_TCP_SERVER_H
