#include "TcpServer.h"


int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_context io_context();
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
		IOEvent::TcpServer s(io_context, ep);
		s.start();
		io_context.run();

	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}


	//t.join();

	return 0;
}
