#include "Accept.h"
#include <boost/system/error_code.hpp>
#include <iostream>
#include "IOLoopThreadPool.h"
#include <glog/logging.h>
#include "IOLoop.h"
namespace IOEvent
{
Acceptor::Acceptor(IOLoop *loop, const Endpoint & endpoint)
	:loop_(loop),
	accept_(*loop->getContext(), endpoint)
{
}

Acceptor::~Acceptor()
{
	accept_.close();
}

void Acceptor::setNewConnectCallback(NewConnectCallback cb)
{
	newConnectCallback_ = std::move(cb);
}

//void Acceptor::accept()
//{
//	accept_.async_accept(socket_, [&](const boost::system::error_code &error)
//	{
//		if (!error)
//		{
//			if (newConnectCallback_) newConnectCallback_(std::move(socket_));
//			else socket_.close();
//		}
//		else
//		{
//			LOG(WARNING) << "accept failed! " << error.message();
//			socket_.close();
//		}
//		accept();
//	});
//	socket_ = ip::tcp::socket(*threadPool_->getNextIOContext());
//}

void Acceptor::accept()
{
	accept_.async_accept([&](const boost::system::error_code &error, ip::tcp::socket &&socket)
	{
		if (!error)
		{
			if (newConnectCallback_) newConnectCallback_(std::move(socket));
			else socket.close();
		}
		else
		{
			LOG(WARNING) << "accept failed! " << error.message();
			socket.close();
		}
		accept();
	});
}
void Acceptor::start()
{
	accept();
}
}
