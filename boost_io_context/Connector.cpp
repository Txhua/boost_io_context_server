#include "Connector.h"
#include "IOLoop.h"
#include <cassert>
#include <glog/logging.h>
#include <boost/asio/connect.hpp>

namespace IOEvent
{

Connector::Connector(IOLoop * loop, const ip::tcp::endpoint & endpoint)
	:loop_(loop),
	socket_(*loop->getContext()),
	serverAddr_(endpoint),
	connect_(false),
	state_(kDisconnected),
	retryDelayMs_(kInitRetryDelayMs)
{
}

Connector::~Connector()
{
	LOG(INFO) << "~Connector[" << this << "]";
	socket_.close();
}

void Connector::start()
{
	connect_ = true;
	loop_->dispatch(std::bind(&Connector::startInThisThread, this));
}

void Connector::stop()
{
	connect_ = false;
	loop_->dispatch(std::bind(&Connector::stopInThisThread, this));
}

void Connector::restart()
{
	loop_->assertInLoopThread();
	setState(kDisconnected);
	retryDelayMs_ = kInitRetryDelayMs;
	connect_ = true;
	startInThisThread();
}

std::string Connector::serverAddress()
{
	return serverAddr_.address().to_string();
}

void Connector::connect()
{
	socket_ = ip::tcp::socket(*loop_->getContext());
	auto self(shared_from_this());
	socket_.async_connect(serverAddr_, [self, this](const boost::system::error_code &error)
	{
		switch (error.value())
		{
			case boost::system::errc::success:
			case boost::system::errc::operation_in_progress:
			case boost::system::errc::interrupted:
			case boost::system::errc::already_connected:
			{
				// 正在连接
				connecting();
				break;
			}
			case boost::system::errc::resource_unavailable_try_again:
			case boost::system::errc::address_in_use:
			case boost::system::errc::address_not_available:
			case boost::system::errc::connection_refused:
			case boost::system::errc::network_unreachable:
			{
				// 尝试重新连接
				retry();
				break;
			}
			case boost::system::errc::permission_denied:
			case boost::system::errc::operation_not_permitted:
			case boost::system::errc::address_family_not_supported:
			case boost::system::errc::connection_already_in_progress:
			case boost::system::errc::bad_file_descriptor:
			case boost::system::errc::bad_address:
			case boost::system::errc::not_a_socket:
			{
				LOG(ERROR) << "connect error in Connector::connect " << error.message();
				socket_.close();
				break;
			}
			default:
			{
				LOG(ERROR) << "Unexpected error in Connector::connect " << error.message();
				socket_.close();
				break;
			}
		}
	});
}

void Connector::retry()
{
	socket_.close();
	setState(kDisconnected);
	if (connect_)
	{
		LOG(INFO) << "Connector::retry - Retry connecting to " << serverAddr_.address().to_string() << " in " << retryDelayMs_ << " milliseconds. ";
		loop_->runAfter(retryDelayMs_ / 1000.0,std::bind(&Connector::startInThisThread, shared_from_this()));
		retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
	}
	else
	{
		LOG(INFO) << "do not connect";
	}
}

void Connector::startInThisThread()
{
	loop_->assertInLoopThread();
	assert(state_ == kDisconnected);
	if (connect_)
	{
		connect();
	}
	else
	{
		LOG(INFO) << "do not connect";
	}
}

void Connector::stopInThisThread()
{
	loop_->assertInLoopThread();
	if (state_ == kConnecting)
	{
		setState(kDisconnected);
		retry();
	}
}

void Connector::connecting()
{
	setState(kConnecting);
	SocketErrorType error;
	boost::system::error_code ec;
	socket_.get_option(error, ec);
	if (ec)
	{
		LOG(ERROR) << "socket_.get_option error : " << ec.message();
		socket_.close();
		return;
	}
	auto errNum = error.value();
	if (errNum)
	{
		LOG(WARNING) << "Connector::handleWrite - SO_ERROR = " << errNum;
		retry();
	}
	else
	{
		setState(kConnected);
		if (connect_)
		{
			newConnectionCallback_(std::move(socket_));
		}
		else
		{
			socket_.close();
		}
	}

}

}
