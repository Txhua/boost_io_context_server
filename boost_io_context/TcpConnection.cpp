#include "TcpConnection.h"
#include "IOLoop.h"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/socket_base.hpp>
#include <glog/logging.h>


namespace IOEvent
{
TcpConnection::TcpConnection(IOLoop *loop, ip::tcp::socket&& socket, const std::string& name)
	:loop_(loop),
	socket_(std::move(socket)),
	name_(name),
	state_(kConnecting)
{
	socket_.set_option(ip::tcp::no_delay(true));
}

TcpConnection::~TcpConnection()
{
	LOG(INFO) << "TcpConnection::~TcpConnection name: " << name_;
	socket_.close();
	assert(state_ == kDisconnected);
}

void TcpConnection::connectEstablished()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnecting);
	setState(kConnected);
	if (connectionCallback_) connectionCallback_(shared_from_this());
	readHeader();
}

void TcpConnection::connectDestroyed()
{
	loop_->assertInLoopThread();
	if (state_ == kConnected)
	{
		setState(kDisconnected);
		connectionCallback_(shared_from_this());
	}
}

void TcpConnection::shutdown()
{
	if (state_ == kConnected)
	{
		setState(kDisconnecting);
		boost::asio::post(socket_.get_executor(), std::bind(&TcpConnection::shutdownInThisThread, shared_from_this()));
	}
}


void TcpConnection::send(Buffer * buf)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInThisThread(buf->retrieveAllAsString());
		}
		else
		{
			loop_->post(std::bind(&TcpConnection::sendInThisThread, shared_from_this(), buf->retrieveAllAsString()));
		}	
	}
}

void TcpConnection::send(std::string &&message)
{
	if (state_ == kConnected)
	{
		if (loop_->isInLoopThread())
		{
			sendInThisThread(message);
		}
		else
		{
			loop_->post(std::bind(&TcpConnection::sendInThisThread, shared_from_this(), std::forward<StringPiece>(message)));
		}		
	}
}


void TcpConnection::sendInThisThread(const StringPiece &str)
{
	loop_->assertInLoopThread();
	if (state_ == kDisconnected)
	{
		LOG(WARNING) << "disconnected, give up writing";
		return;
	}
	auto write_in_progress = (outputBuffer_.readableBytes() == 0) ? true : false;
	outputBuffer_.append(str.data(), str.size());
	if (write_in_progress)
	{
		write();
	}
}

void TcpConnection::shutdownInThisThread()
{
	loop_->assertInLoopThread();
	if (outputBuffer_.readableBytes() == 0)
	{
		// 确保缓冲区的数据已被发送完
		boost::system::error_code error;
		socket_.shutdown(socket_base::shutdown_type::shutdown_send, error);
		if (error)
		{
			LOG(ERROR) << "socket_.shutdown, error message : " << error.message();
		}
	}
}

void TcpConnection::handleClose()
{
	loop_->assertInLoopThread();
	assert(state_ == kConnected || state_ == kDisconnecting);
	setState(kDisconnected);
	TcpConnectionPtr guardThis(shared_from_this());
	if (connectionCallback_) connectionCallback_(guardThis);
	if (closeCallback_) closeCallback_(guardThis);
}

void TcpConnection::readHeader()
{
	auto self(shared_from_this());
	boost::asio::async_read(socket_, boost::asio::buffer(inputBuffer_.beginWrite(), sizeof(int32_t)), [this, self](const boost::system::error_code &error, size_t byte)
	{
		if (error)
		{
			LOG(WARNING) << "readHeader() error! " << name_ << " error message: " << error.message();
			handleClose();
		}
		else
		{
			inputBuffer_.hasWritten(byte);
			auto len = inputBuffer_.peekInt32();
			inputBuffer_.ensureWritableBytes(len);
			readBody(len);
		}
	});
}

void TcpConnection::readBody(size_t len)
{
	auto self(shared_from_this());
	boost::asio::async_read(socket_, boost::asio::buffer(inputBuffer_.beginWrite(), len), [this, self](const boost::system::error_code &error, size_t byte)
	{
		if (error)
		{
			LOG(WARNING) << "readBody() error! " << name_ << " error message: " << error.message();
			handleClose();
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
			else
			{
				if (state_ == kDisconnecting)
				{
					shutdownInThisThread();
				}
			}
		}
		else
		{
			// boost::system::errc::operation_would_block 对方的缓冲区可能已被写满,不是真正的错误
			if (error != boost::system::errc::operation_would_block)
			{
				LOG(ERROR) << "TcpConnection::write" << name_;
				// socket已断开连接
				if (error == boost::system::errc::connection_reset || error == boost::system::errc::broken_pipe)
				{
					LOG(ERROR) << "TcpConnection socket unlink" << name_;
				}
			}
		}
	});
}


}
