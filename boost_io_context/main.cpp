#include "TcpServer.h"
#include <boost/asio.hpp>
#include "Dispatcher.h"
#include "ProtobufCodec.h"
#include "TcpConnection.h"
#include "IOLoop.h"
#include "CurrentThread.h"
#include "Logging.h"
#include <glog/logging.h>

using namespace IOEvent;

using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		Logging::instance()->glogInitializer();
		LOG(INFO) << "main thread tid: " << CurrentThread::tid();
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
		auto loop = Singleton<IOLoop>::instance();
		IOEvent::TcpServer s(&*loop, ep);
		s.setThreadNum(5);
		s.setMessageCallback([&](const TcpConnectionPtr &conn, Buffer *buf) {});
		s.setConnectionCallback([&](const TcpConnectionPtr &conn) {});
		s.start();
		loop->loop();
		google::ShutdownGoogleLogging();
	}
	catch (std::exception &e)
	{
		LOG(ERROR) << e.what();
	}
	return 0;
}
