#include "ProtobufCodec.h"
#include <google/protobuf/message.h>
#include "Buffer.h"
#include "TcpConnection.h"
#include <cassert>
#include <zlib.h>
#include <glog/logging.h>

namespace IOEvent
{
ProtobufCodec::ProtobufCodec(ProtobufMessageCallback cb)
	:messageCallback_(std::move(cb))
{

}
void ProtobufCodec::onMessage(const TcpConnectionPtr & conn, Buffer * buf)
{
	while (buf->readableBytes() >= kMinMessageLen + kHeaderLen)
	{
		int32_t bodyLen = buf->peekInt32();
		if (bodyLen > kMaxMessageLen || bodyLen < kMinMessageLen)
		{
			conn->shutdown();
			return;
		}
		else if (buf->readableBytes() >= bodyLen + kHeaderLen)
		{
			MessagePtr message = parse(buf->peek() + kHeaderLen, bodyLen);
			if (message)
			{
				messageCallback_(conn, message);
				buf->retrieve(bodyLen + kHeaderLen);
			}
		}
	}
}
void ProtobufCodec::send(const TcpConnectionPtr & conn, google::protobuf::Message * message)
{
	Buffer buff;
	const std::string &typeName = message->GetTypeName();
	int32_t nameLen = static_cast<int32_t>(typeName.size() + 1);
	buff.appendInt32(nameLen);
	buff.append(typeName.c_str(), nameLen);
	int byteSize = message->ByteSize();
	buff.ensureWritableBytes(byteSize);
	uint8_t *start = reinterpret_cast<uint8_t *>(buff.beginWrite());
	uint8_t *end = message->SerializeWithCachedSizesToArray(start);
	if (end - start != byteSize)
	{
		LOG(FATAL) << "google SerializeWithCachedSizesToArray error";
	}
	buff.hasWritten(byteSize);
	int32_t checkSum = static_cast<int32_t>(::adler32(1, reinterpret_cast<const Bytef*>(buff.peek()), static_cast<int>(buff.readableBytes())));
	buff.appendInt32(checkSum);
	assert(buff.readableBytes() == sizeof(nameLen) + nameLen + byteSize + sizeof(checkSum));
	int32_t len = boost::asio::detail::socket_ops::host_to_network_long(static_cast<int32_t>(buff.readableBytes()));
	buff.prepend(&len, sizeof(len));
	conn->send(&buff);
}
google::protobuf::Message * ProtobufCodec::createMessage(const std::string & typeName)
{
	google::protobuf::Message *message = nullptr;
	const google::protobuf::Descriptor *descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
	if (descriptor)
	{
		const google::protobuf::Message *protoType = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (protoType)
		{
			message = protoType->New();
		}
	}
	return message;
}
MessagePtr ProtobufCodec::parse(const char * data, size_t len)
{
	MessagePtr message;
	// 检测校验位
	int32_t expectedCheckSum = networkToHost32(data + len - kHeaderLen);
	int32_t checkSum = static_cast<int32_t>(::adler32(1, reinterpret_cast<const Bytef *>(data), static_cast<int>(len - kHeaderLen)));
	if (checkSum == expectedCheckSum)
	{
		int32_t nameLen = networkToHost32(data);
		if (nameLen >= 2 && nameLen <= len - 2 * kHeaderLen)
		{
			std::string typeName(data + kHeaderLen, data + kHeaderLen + nameLen - 1);
			message.reset(createMessage(typeName));
			if (message)
			{
				//parse from buff
				const char *buf = data + kHeaderLen + nameLen;
				size_t dataLen = len - nameLen - 2 * kHeaderLen;
				message->ParseFromArray(buf, static_cast<int>(dataLen));
			}
		}
	}
	return message;
}

int32_t ProtobufCodec::networkToHost32(const char * buf)
{
	int32_t be32 = 0;
	::memcpy(&be32, buf, sizeof(int32_t));
	return boost::asio::detail::socket_ops::network_to_host_long(be32);
}

}
