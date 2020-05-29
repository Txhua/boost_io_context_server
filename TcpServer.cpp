#include "TcpServer.h"
#include "Accept.h"
#include "TcpConnection.h"
#include <algorithm>
#include <functional>
#include <glog/logging.h>

namespace IOEvent
{
TcpServer::TcpServer(io_context & ios, const ip::tcp::endpoint & endpoint)
	:accept_(std::make_unique<Acceptor>(ios, endpoint)),
	started_(false),
	ipPort_(endpoint.address().to_string() + ":" + std::to_string(endpoint.port())),
	nextConnId_(1)
{
	accept_->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
	LOG(INFO) << "tcp server quit";
}

void TcpServer::start()
{
	if (!started_.load(std::memory_order_acquire))
	{
		LOG(INFO) << "tcp server start";
		started_.store(true, std::memory_order_release);
		accept_->start();
	}
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	LOG(INFO) << "dicconnection :" << conn->name();
	connections_.erase(conn->name());
}

void TcpServer::newConnection(ip::tcp::socket && socket)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "%s#%d", ipPort_.c_str(), nextConnId_);
	++nextConnId_;
	auto ios = socket.get_executor();
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(std::move(socket), buf);
	connections_[buf] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	// 投递给所属的io_context,去执行conn的IO事件
	boost::asio::post(ios, std::bind(&TcpConnection::connectEstablished, conn));
}

}
