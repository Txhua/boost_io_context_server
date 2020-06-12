#include "TcpServer.h"
#include "Accept.h"
#include "IOLoop.h"
#include "TcpConnection.h"
#include "IOLoopThreadPool.h"
#include <glog/logging.h>
#include <functional>

namespace IOEvent
{
TcpServer::TcpServer(IOLoop *loop, const ip::tcp::endpoint & endpoint)
	:baseLoop_(loop),
	accept_(std::make_unique<Acceptor>(loop, endpoint)),
	threadPool_(std::make_shared<IOLoopThreadPool>(loop)),
	started_(false),
	ipPort_(endpoint.address().to_string()),
	nextConnId_(1)
{
	accept_->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
	baseLoop_->assertInLoopThread();
	LOG(WARNING) << "server exit :" << this;
	for (auto &item : connections_)
	{
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->dispatch(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}

void TcpServer::start()
{
	if (!started_)
	{
		LOG(INFO) << "server start ";
		started_ = true;
		threadPool_->run();
		accept_->start();
	}
}

void TcpServer::setThreadNum(int numThreads)
{
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	baseLoop_->dispatch(std::bind(&TcpServer::removeConnectionInThisThread, this, conn));
}

void TcpServer::removeConnectionInThisThread(const TcpConnectionPtr & conn)
{
	baseLoop_->assertInLoopThread();
	LOG(INFO) << "TcpServer::removeConnectionInThisThread : " << conn->name();
	size_t n = connections_.erase(conn->name());
	(void)n;
	assert(n == 1);
	auto *ioLoop = conn->getLoop();
	ioLoop->post(std::bind(&TcpConnection::connectDestroyed, conn));
}

void TcpServer::newConnection(ip::tcp::socket && socket)
{
	baseLoop_->assertInLoopThread();
	char buf[64];
	auto *ioLoop = threadPool_->getNextIOLoop();
	ip::tcp::socket peerSocket(*ioLoop->getContext());
	peerSocket.assign(ip::tcp::v4(), socket.release());
	snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
	++nextConnId_;
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, std::move(peerSocket), buf);
	connections_[buf] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setWriteCompleteCallback(writeCompleteCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	ioLoop->dispatch(std::bind(&TcpConnection::connectEstablished, conn));
}

}
