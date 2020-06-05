#include "Codec.h"
#include <glog/logging.h>
#include "Buffer.h"
#include "TcpConnection.h"
namespace IOEvent
{
LengthHeaderCodec::LengthHeaderCodec(StringMessageCallback cb)
	:messageCallback_(std::move(cb))
{}

LengthHeaderCodec::~LengthHeaderCodec()
{
}

void LengthHeaderCodec::onMessage(const TcpConnectionPtr& conn, Buffer* message)
{
	while (message->readableBytes() > kHeaderLen)
	{
		int32_t bodyLen = message->peekInt32();
		if (bodyLen > kMaxMessageLen || bodyLen < kMinMessageLen)
		{
			LOG(ERROR) << "invalid body len! " << conn->name();
			conn->shutdown();
			return;
		}
		else if (message->readableBytes() >= static_cast<size_t>(bodyLen + kHeaderLen))
		{
			message->retrieve(kHeaderLen);
			std::string s(message->peek(), bodyLen);
			messageCallback_(conn, s);
			message->retrieve(bodyLen);
		}
		else
		{
			break;
		}
	}
}

void LengthHeaderCodec::send(const TcpConnectionPtr& conn, const StringPiece& str)
{
	send(conn, str.data(), str.size());
}

void LengthHeaderCodec::send(const TcpConnectionPtr& conn, const void* data, size_t len)
{
	send(conn, static_cast<const char*>(data), len);
}

void LengthHeaderCodec::send(const TcpConnectionPtr& conn, const char* data, size_t len)
{
	//auto *outBuff = conn->outputBuffer();
	//bool write_in_progress = (outBuff->readableBytes() == 0) ? true : false;
	//outBuff->append(data, len);
	//outBuff->prependInt32(static_cast<int32_t>(len));
	//if (write_in_progress)
	//{
	//	// 发送数据
	//	conn->send();
	//}
}
}
