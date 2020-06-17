#include "TcpClient.h"
#include "IOLoop.h"
#include "TcpConnection.h"
#include "Connector.h"
#include "Types.h"
#include <glog/logging.h>
#include <cassert>

namespace IOEvent
{

namespace detail
{

void removeConnection(IOLoop* loop, const TcpConnectionPtr& conn)
{
	loop->dispatch(std::bind(&TcpConnection::connectDestroyed, conn));
}

}

TcpClient::TcpClient(IOLoop * loop, const Endpoint & serverAddr, const std::string & name)
	:loop_(loop),
	connector_(std::make_shared<Connector>(loop, serverAddr)),
	name_(name),
	retry_(false),
	connect_(false)
{
	connector_->setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient()
{
	LOG(INFO) << "TcpClient::~TcpClient[" << name_ << "] - connector " << get_pointer(connector_);
	TcpConnectionPtr conn;
	bool unique = false;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		unique = connection_.unique();
		conn = connection_;
	}
	if (conn)
	{
		assert(loop_ == conn->getLoop());
		CloseCallback cb = std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
		loop_->dispatch(std::bind(&TcpConnection::setCloseCallback, conn, cb));
		if (unique)
		{
			//conn->forceClose();
		}
	}
	else
	{
		connector_->stop();
	}
}

void TcpClient::connect()
{
	LOG(INFO) << "TcpClient::connect[" << name_ << "] - connecting to " << connector_->serverAddress();
	connect_ = true;
	connector_->start();
}

void TcpClient::disconnect()
{
	connect_ = false;
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if (connection_)
		{
			connection_->shutdown();
		}
	}
}

void TcpClient::stop()
{
	connect_ = false;
	connector_->stop();
}

bool TcpClient::enableRetry()
{
	return retry_ = true;
}

TcpConnectionPtr TcpClient::connection() const
{
	std::unique_lock<std::mutex> lock(mutex_);
	return connection_;
}

void TcpClient::newConnection(Socket && socket)
{
	loop_->assertInLoopThread();
	char buf[32];
	snprintf(buf, sizeof(buf), ":%s#%d", connector_->serverAddress().data(), nextConnId_);
	++nextConnId_;
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, std::move(socket), buf);
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
	{
		std::unique_lock<std::mutex> lock(mutex_);
		connection_ = conn;
	}
	conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr & conn)
{
	loop_->assertInLoopThread();
	assert(loop_ == conn->getLoop());

	{
		std::unique_lock<std::mutex> lock(mutex_);
		assert(connection_ == conn);
		connection_.reset();
	}

	loop_->post(std::bind(&TcpConnection::connectDestroyed, conn));
	if (retry_ && connect_)
	{
		LOG(INFO) << "TcpClient::connect[" << name_ << "] - Reconnecting to " << connector_->serverAddress();
		connector_->restart();
	}
}

}
