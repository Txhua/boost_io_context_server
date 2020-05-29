#include "Buffer.h"
namespace IOEvent
{

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;


Buffer::Buffer(size_t initialSize)
	: buffer_(kCheapPrepend + initialSize),
	readerIndex_(kCheapPrepend),
	writerIndex_(kCheapPrepend)
{
	assert(readableBytes() == 0);
	assert(writableBytes() == initialSize);
	assert(prependableBytes() == kCheapPrepend);
}
void Buffer::swap(Buffer & rhs)
{
	buffer_.swap(rhs.buffer_);
	std::swap(readerIndex_, rhs.readerIndex_);
	std::swap(writerIndex_, rhs.writerIndex_);
}
void Buffer::retrieve(size_t len)
{
	assert(len <= readableBytes());
	if (len < readableBytes())
	{
		readerIndex_ += len;
	}
	else
	{
		retrieveAll();
	}
}
void Buffer::retrieveUntil(const char * end)
{
	assert(peek() <= end);
	assert(end <= beginWrite());
	retrieve(end - peek());
}
void Buffer::retrieveAll()
{
	readerIndex_ = kCheapPrepend;
	writerIndex_ = kCheapPrepend;
}
std::string Buffer::retrieveAllAsString()
{
	return retrieveAsString(readableBytes());
}
std::string Buffer::retrieveAsString(size_t len)
{
	assert(len <= readableBytes());
	std::string result(peek(), len);
	retrieve(len);
	return result;
}
StringPiece Buffer::toStringPiece() const
{
	return StringPiece(peek(), static_cast<int>(readableBytes()));
}
void Buffer::append(const StringPiece & str)
{
	append(str.data(), str.size());
}
void Buffer::append(const char * data, size_t len)
{
	ensureWritableBytes(len);
	std::copy(data, data + len, beginWrite());
	hasWritten(len);
}

void Buffer::append(const void * data, size_t len)
{
	append(static_cast<const char*>(data), len);
}

void Buffer::ensureWritableBytes(size_t len)
{
	if (writableBytes() < len)
	{
		makeSpace(len);
	}
	assert(writableBytes() >= len);
}


char * Buffer::beginWrite()
{
	return begin() + writerIndex_;
}

const char * Buffer::beginWrite() const
{
	return begin() + writerIndex_;
}

void Buffer::hasWritten(size_t len)
{
	assert(len <= writableBytes());
	writerIndex_ += len;
}

void Buffer::unwrite(size_t len)
{
	assert(len <= readableBytes());
	writerIndex_ -= len;
}

void Buffer::appendInt32(int32_t x)
{
	int32_t be32 = boost::asio::detail::socket_ops::host_to_network_long(x);
	append(&be32, sizeof(be32));
}

void Buffer::appendInt16(int16_t x)
{
	int16_t be16 = boost::asio::detail::socket_ops::host_to_network_short(x);
	append(&be16, sizeof(be16));
}

void Buffer::appendInt8(int8_t x)
{
	append(&x, sizeof(x));
}

int32_t Buffer::readInt32()
{
	int32_t result = peekInt32();
	retrieveInt32();
	return result;
}

int16_t Buffer::readInt16()
{
	int16_t result = peekInt16();
	retrieveInt16();
	return result;
}

int8_t Buffer::readInt8()
{
	int8_t result = peekInt8();
	retrieveInt8();
	return result;
}

int32_t Buffer::peekInt32() const
{
	assert(readableBytes() >= sizeof(int32_t));
	int32_t be32 = 0;
	::memcpy(&be32, peek(), sizeof(be32));
	return boost::asio::detail::socket_ops::network_to_host_long(be32);
}

int16_t Buffer::peekInt16() const
{
	assert(readableBytes() >= sizeof(int16_t));
	int16_t be16 = 0;
	::memcpy(&be16, peek(), sizeof be16);
	return boost::asio::detail::socket_ops::network_to_host_short(be16);
}

int8_t Buffer::peekInt8() const
{
	assert(readableBytes() >= sizeof(int8_t));
	int8_t x = *peek();
	return x;
}

void Buffer::prependInt32(int32_t x)
{
	int32_t be32 = boost::asio::detail::socket_ops::host_to_network_long(x);
	prepend(&be32, sizeof be32);
}

void Buffer::prependInt16(int16_t x)
{
	int16_t be16 = boost::asio::detail::socket_ops::host_to_network_short(x);
	prepend(&be16, sizeof(be16));
}

void Buffer::prependInt8(int8_t x)
{
	prepend(&x, sizeof(x));
}

void Buffer::prepend(const void * data, size_t len)
{
	assert(len <= prependableBytes());
	readerIndex_ -= len;
	const char* d = static_cast<const char*>(data);
	std::copy(d, d + len, begin() + readerIndex_);
}

void Buffer::shrink(size_t reserve)
{
	Buffer other;
	other.ensureWritableBytes(readableBytes() + reserve);
	other.append(toStringPiece());
	swap(other);
}

size_t Buffer::internalCapacity() const
{
	return buffer_.capacity();
}
char * Buffer::begin()
{
	return &*buffer_.begin();
}
const char * Buffer::begin() const
{
	return &*buffer_.begin();
}
void Buffer::makeSpace(size_t len)
{
	if (writableBytes() + prependableBytes() < len + kCheapPrepend)
	{
		buffer_.resize(writerIndex_ + len);
	}
	else
	{
		assert(kCheapPrepend < readerIndex_);
		size_t readable = readableBytes();
		std::copy(begin() + readerIndex_,
			begin() + writerIndex_,
			begin() + kCheapPrepend);
		readerIndex_ = kCheapPrepend;
		writerIndex_ = readerIndex_ + readable;
		assert(readable == readableBytes());
	}
}

}
