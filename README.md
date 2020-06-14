// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
// Author: Txhua
IOEvent 是基于boost::asio 封装的一个通用的Tcp网络库,不支持UDP
包含的功能:
	1、c++11的线程池
	2、时间队列
	3、TcpServer
	3、TcpClient支持断线重连
用法:
	// 多线程的TcpServer, 如果需要单线程setThreadNum函数注释即可;
	IOEvent::IOLoop loop;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
	IOEvent::TcpServer s(&loop, ep);
	s.setThreadNum(5);
	s.setMessageCallback([&](const TcpConnectionPtr &conn, Buffer *buf) {});
	s.setConnectionCallback([&](const TcpConnectionPtr &conn) {});
	s.start();
	loop.loop();
	
这是我业余时间的一个作品,有些细节还需要优,往后有时间慢慢改进吧