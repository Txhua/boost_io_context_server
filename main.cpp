#include "TcpServer.h"
#include <iostream>
#include <boost/asio.hpp>
#include <glog/logging.h>

int main(int argc, char* argv[])
{
	
	try
	{
		// 设置输出路径
		FLAGS_log_dir = "./log";
		// 应用程序的名称
		google::InitGoogleLogging("TcpServer");
		//设置级别高于 google::INFO 的日志同时输出到屏幕
		google::SetStderrLogging(google::GLOG_INFO); 
		 //设置输出到屏幕的日志显示相应颜色
		FLAGS_colorlogtostderr = true;   
		//设置 google::INFO 级别的日志存储路径和文件名前缀
		google::SetLogDestination(google::GLOG_INFO, "./log/INFO_"); 
		//设置 google::WARNING 级别的日志存储路径和文件名前缀
		google::SetLogDestination(google::GLOG_WARNING,"./log/WARNING_");
		//设置 google::ERROR 级别的日志存储路径和文件名前缀
		google::SetLogDestination(google::GLOG_ERROR, "./log/ERROR_");
		//缓冲日志输出，默认为30秒，此处改为立即输出
		FLAGS_logbufsecs = 0;   
		//最大日志大小为 100MB
		FLAGS_max_log_size = 100; 
		//当磁盘被写满时，停止日志输出
		FLAGS_stop_logging_if_full_disk = true; 
		//设置文件名扩展，如平台？或其它需要区分的信息
		google::SetLogFilenameExtension("server_"); 
		//捕捉 core dumped
		google::InstallFailureSignalHandler();
		
		//默认捕捉 SIGSEGV 信号信息输出会输出到 stderr，可以通过下面的方法自定义输出>方式：
	    //google::InstallFailureWriter(&SignalHandle); 
		LOG(INFO) << "main thread tid" << std::this_thread::get_id();
		boost::asio::io_context io_context;
		boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work(boost::asio::make_work_guard(io_context));
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
		IOEvent::TcpServer s(io_context, ep);
		s.setThreadNum(5);
		s.start();
		io_context.run();
		//google::ShutdownGoogleLogging();
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
	}


	//t.join();

	return 0;
}
