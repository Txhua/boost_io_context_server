//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_ACCEPT_H
#define _IOEVENT_ACCEPT_H


#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <functional>

namespace IOEvent
{
class IOLoopThreadPool;
class IOLoop;
class Acceptor final : public boost::noncopyable
{
	using Socket = boost::asio::ip::tcp::socket;
	using acceptor = boost::asio::ip::tcp::acceptor;
	using NewConnectCallback = std::function<void(Socket &&sock)>;
	using Endpoint = boost::asio::ip::tcp::endpoint;
public:
	explicit Acceptor(IOLoop *loop, const Endpoint &endpoint);
	~Acceptor();
	void setNewConnectCallback(NewConnectCallback cb);
	void start();
private:
	void accept();
private:
	IOLoop *loop_;
	acceptor accept_;
	NewConnectCallback newConnectCallback_;
};

}





#endif // !_IOEVENT_ACCEPT_H
