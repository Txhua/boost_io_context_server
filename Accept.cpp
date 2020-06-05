#include "Accept.h"
#include <boost/system/error_code.hpp>
#include <iostream>
#include "IOContextThreadPool.h"
#include <glog/logging.h>
namespace IOEvent
{
Acceptor::Acceptor(io_context & ios, const Endpoint & endpoint)
	:accept_(ios, endpoint),
	socket_(ios),
	ioContextPool_(std::make_unique<IOContextThreadPool>(ios))
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

void Acceptor::setThreadNum(int numThreads)
{
	ioContextPool_->setThreadNum(numThreads);
}

void Acceptor::accept()
{
	accept_.async_accept(socket_, [&](const boost::system::error_code &error)
	{
		if (!error)
		{
			if (newConnectCallback_) newConnectCallback_(std::move(socket_));
			else socket_.close();
		}
		else
		{
			LOG(WARNING) << "accept failed! " << error.message();
			socket_.close();
		}
		accept();
	});
	socket_ = ip::tcp::socket(*ioContextPool_->getNextIOContext());
}
void Acceptor::start()
{
	ioContextPool_->run();
	accept();
}
}
