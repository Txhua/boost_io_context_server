#include "TcpConnection.h"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/socket_base.hpp>
#include <glog/logging.h>


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
	socket_.close();
}

void TcpConnection::connectEstablished()
{
	if (connectionCallback_) connectionCallback_(shared_from_this());
	readHeader();
}

void TcpConnection::shutdown()
{
	boost::asio::post(socket_.get_executor(), std::bind(&TcpConnection::shutdownInThisThread, shared_from_this()));
}

void TcpConnection::send()
{
	boost::asio::post(socket_.get_executor(), std::bind(&TcpConnection::write, shared_from_this()));
}

void TcpConnection::shutdownInThisThread()
{
	socket_.shutdown(socket_base::shutdown_type::shutdown_send);
}

void TcpConnection::readHeader()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket_, boost::asio::buffer(inputBuffer_.beginWrite(), sizeof(int32_t)), [this, self](const boost::system::error_code &error, size_t byte)
	{
		if (error)
		{
			LOG(WARNING) << "readHeader() error! " << name_ << " error message: " << error.message();
			shutdownInThisThread();
			closeCallback_(shared_from_this());
		}
		else
		{
			inputBuffer_.hasWritten(byte);
			auto len = inputBuffer_.peekInt32();
			inputBuffer_.ensureWritableBytes(len);
			readBody();
		}
	});
}

void TcpConnection::readBody()
{
	auto self(shared_from_this());
	auto len = inputBuffer_.peekInt32();
	boost::asio::async_read(socket_, boost::asio::buffer(inputBuffer_.beginWrite(), len), [this, self](const boost::system::error_code &error, size_t byte)
	{
		if (error)
		{
			LOG(WARNING) << "readBody() error! " << name_ << " error message: " << error.message();
			shutdownInThisThread();
			closeCallback_(shared_from_this());
		}
		else
		{
			inputBuffer_.hasWritten(byte);
			messageCallback_(shared_from_this(), &inputBuffer_);
			readHeader();
		}
	});
}

void TcpConnection::write()
{
	auto self(shared_from_this());
	boost::asio::async_write(socket_, boost::asio::buffer(outputBuffer_.peek(), outputBuffer_.readableBytes()),
		[this, self](const boost::system::error_code &error, std::size_t byte)
	{
		if (!error)
		{
			outputBuffer_.retrieve(byte);
			if (outputBuffer_.readableBytes() != 0)
			{
				write();
			}
		}
		else
		{
			shutdownInThisThread();
			outputBuffer_.retrieveAll();
			closeCallback_(shared_from_this());
		}
	});
}


}
