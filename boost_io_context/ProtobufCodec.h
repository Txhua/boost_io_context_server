#ifndef _IOEVENT_PROTOBUF_CODEC_H
#define _IOEVENT_PROTOBUF_CODEC_H
#include <memory>
#include "Callbacks.h"
namespace google
{
namespace protobuf
{
class Message;
}
}

namespace IOEvent
{

using MessagePtr = std::shared_ptr<google::protobuf::Message>;
class ProtobufCodec
{
	using ProtobufMessageCallback = std::function<void(const TcpConnectionPtr&,const MessagePtr&)>;
public:
	explicit ProtobufCodec(ProtobufMessageCallback cb);
	void onMessage(const TcpConnectionPtr &conn, Buffer *message);
	void send(const TcpConnectionPtr &conn, google::protobuf::Message *message);
private:
	google::protobuf::Message *createMessage(const std::string &typeName);
	MessagePtr parse(const char *data, size_t len);
	int32_t networkToHost32(const char *buf);
private:
	ProtobufMessageCallback messageCallback_;
	const static int kHeaderLen = sizeof(int32_t);       
	const static int kMinMessageLen = 2 * kHeaderLen + 2; 
	const static int kMaxMessageLen = 64 * 1024 * 1024; 
};

}


#endif // !_IOEVENT_PROTOBUF_CODEC_H

