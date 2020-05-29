#ifndef _IOEVENT_BUFFER_H
#define _IOEVENT_BUFFER_H

#include <algorithm>
#include <vector>
#include <assert.h>
#include "StringPiece.h"
#include <boost/asio/detail/socket_ops.hpp>


namespace IOEvent
{


/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer final
{
public:
	static const size_t kCheapPrepend = 8;
	static const size_t kInitialSize = 1024;

	explicit Buffer(size_t initialSize = kInitialSize);
	void swap(Buffer& rhs);
	size_t readableBytes() const { return writerIndex_ - readerIndex_; }
	size_t writableBytes() const { return buffer_.size() - writerIndex_; }
	size_t prependableBytes() const { return readerIndex_; }
	const char* peek() const { return begin() + readerIndex_; }
	void retrieve(size_t len);
	void retrieveUntil(const char* end);
	void retrieveInt32() { retrieve(sizeof(int32_t)); }
	void retrieveInt16() { retrieve(sizeof(int16_t)); }
	void retrieveInt8() { retrieve(sizeof(int8_t)); }
	void retrieveAll();
	std::string retrieveAllAsString();
	std::string retrieveAsString(size_t len);
	StringPiece toStringPiece() const;
	void append(const StringPiece& str);
	void append(const char *data, size_t len);
	void append(const void *data, size_t len);
	void ensureWritableBytes(size_t len);
	char* beginWrite();
	const char* beginWrite() const;
	void hasWritten(size_t len);
	void unwrite(size_t len);
	void appendInt32(int32_t x);
	void appendInt16(int16_t x);
	void appendInt8(int8_t x);
	int32_t readInt32();
	int16_t readInt16();
	int8_t readInt8();
	int32_t peekInt32() const;
	int16_t peekInt16() const;
	int8_t peekInt8() const;
	void prependInt32(int32_t x);
	void prependInt16(int16_t x);
	void prependInt8(int8_t x);
	void prepend(const void * data, size_t len);
	void shrink(size_t reserve);
	size_t internalCapacity() const;
private:
	char* begin();
	const char* begin() const;
	void makeSpace(size_t len);
private:
	std::vector<char> buffer_;
	size_t readerIndex_;
	size_t writerIndex_;
};

}

#endif  // _IOEVENT_BUFFER_H
