﻿//
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
#include <memory>

namespace IOEvent
{
using namespace boost::asio;
class IOLoopThreadPool;
class IOLoop;
class Acceptor final : public boost::noncopyable
{
	using NewConnectCallback = std::function<void(ip::tcp::socket &&sock)>;
	using Endpoint = ip::tcp::endpoint;
public:
	explicit Acceptor(IOLoop *loop, const Endpoint &endpoint);
	~Acceptor();
	void setNewConnectCallback(NewConnectCallback cb);
	void start();
private:
	void accept();
private:
	IOLoop *loop_;
	ip::tcp::acceptor accept_;
	NewConnectCallback newConnectCallback_;
};

}





#endif // !_IOEVENT_ACCEPT_H
