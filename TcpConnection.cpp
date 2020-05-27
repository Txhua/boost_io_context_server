#include "TcpConnection.h"
#include <iostream>

namespace IOEvent
{
TcpConnection::TcpConnection(ip::tcp::socket&& socket, const std::string& name)
	:socket_(std::move(socket)),
	name_(name)
{
	socket_.set_option(ip::tcp::no_delay(true));
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::connectEstablished()
{
	if (connectionCallback_) connectionCallback_(shared_from_this());
	read();
}

void TcpConnection::read()
{
	auto self(shared_from_this());

	socket_.async_read_some(boost::asio::buffer(buffer_), [this, self](const boost::system::error_code &err, size_t len)
	{
		if (err)
		{
			closeCallback_(shared_from_this());
		}
		else
		{
			messageCallback_(shared_from_this());
			read();
		}
	});
}

void TcpConnection::write()
{
}
}
