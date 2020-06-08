#include "TcpServer.h"
#include "Accept.h"
#include "TcpConnection.h"
#include <glog/logging.h>
#include <functional>
namespace IOEvent
{
TcpServer::TcpServer(io_context & ios, const ip::tcp::endpoint & endpoint)
	:baseIoContext_(ios),
	accept_(std::make_unique<Acceptor>(ios, endpoint)),
	started_(false),
	ipPort_(endpoint.address().to_string()),
	nextConnId_(1)
{
	accept_->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
	LOG(WARNING) << "server exit" << this;
}

void TcpServer::start()
{
	if (!started_.load(std::memory_order_acquire))
	{
		LOG(INFO) << "server start";
		started_.store(true, std::memory_order_release);
		accept_->start();
	}
}

void TcpServer::setThreadNum(int numThreads)
{
	accept_->setThreadNum(numThreads);
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	boost::asio::post(baseIoContext_, std::bind(&TcpServer::removeConnectionInThisThread, this, conn));
}

void TcpServer::removeConnectionInThisThread(const TcpConnectionPtr & conn)
{
	LOG(INFO) << "TcpServer::removeConnectionInThisThread : " << conn->name();
	size_t n = connections_.erase(conn->name());
	(void)n;
	assert(n == 1);
	boost::asio::post(conn->getIoService(), std::bind(&TcpConnection::connectDestroyed, conn));
}

void TcpServer::newConnection(ip::tcp::socket && socket)
{
	char buf[64];
	snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
	++nextConnId_;
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(std::move(socket), buf);
	connections_[buf] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	// 投递给所属的io_context,去执行conn的IO事件
	auto ios = conn->getIoService();
	boost::asio::post(std::move(ios), std::bind(&TcpConnection::connectEstablished, conn));
}

}
