﻿#ifndef _IOEVENT_TCP_SERVER_H
#define _IOEVENT_TCP_SERVER_H

#include "Callbacks.h"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <atomic>
#include <map>
namespace IOEvent
{
using namespace boost::asio;
class Acceptor;

class TcpServer
{
	using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
public:
	TcpServer(io_context &ios, const ip::tcp::endpoint &endpoint);
	~TcpServer();
	void start();
	const std::string &ipPort()const { return ipPort_; }
	void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
	void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
	void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }
	void setThreadNum(int numThreads);
private:
	void removeConnection(const TcpConnectionPtr &conn);
	void removeConnectionInThisThread(const TcpConnectionPtr &conn);
	void newConnection(ip::tcp::socket &&socket);
private:
	io_context &baseIoContext_;
	uint32_t nextConnId_;
	const std::string ipPort_;
	std::atomic<bool> started_;
	std::unique_ptr<Acceptor> accept_;
	ConnectionCallback connectionCallback_;
	MessageCallback messageCallback_;
	WriteCompleteCallback writeCompleteCallback_;
	ConnectionMap connections_;
};

}




#endif // !_IOEVENT_TCP_SERVER_H
