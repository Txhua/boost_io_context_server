#ifndef _IOEVENT_CODEC_H
#define _IOEVENT_CODEC_H

#include <boost/noncopyable.hpp>
#include "Callbacks.h"
#include "Buffer.h"
#include "TcpConnection.h"
#include "StringPiece.h"
#include <glog/logging.h>

namespace IOEvent
{

class LengthHeaderCodec : public boost::noncopyable
{
	using StringMessageCallback = std::function<void(const TcpConnectionPtr&, std::string& message)>;
public:
	LengthHeaderCodec(StringMessageCallback cb);
	~LengthHeaderCodec();
	void onMessage(const TcpConnectionPtr &conn, Buffer *message);
	void send(const TcpConnectionPtr &conn, const StringPiece &str);
	void send(const TcpConnectionPtr &conn, const void *data, size_t len);
	void send(const TcpConnectionPtr &conn, const char *data, size_t len);
private:
	StringMessageCallback messageCallback_;
	const static int kHeaderLen = sizeof(int32_t);
	const static int kMaxMessageLen = 65536;
	const static int kMinMessageLen = 0;
};
}




#endif // !_IOEVENT_CODEC_H
