#ifndef _IOEVENT_DISPATCHER_H
#define _IOEVENT_DISPATCHER_H

#include <boost/noncopyable.hpp>
#include <google/protobuf/message.h>
#include <map>
#include "Callbacks.h"
#include "Types.h"

namespace IOEvent
{
using MessagePtr = std::shared_ptr<google::protobuf::Message>;
class Callback : public boost::noncopyable
{
public:
	virtual ~Callback() = default;
	virtual void onMessage(const TcpConnectionPtr &conn, const MessagePtr &message) const = 0;
};

template <typename _Type>
class CallbackT : public Callback
{
public:
	using ProtobufMessageCallback = std::function<void(const TcpConnectionPtr &conn, const std::shared_ptr<_Type>&)>;
	CallbackT(ProtobufMessageCallback cb)
		:protobufMessageCallback_(std::move(cb))
	{}

	void onMessage(const TcpConnectionPtr &conn, const MessagePtr &message) const override
	{
		std::shared_ptr<_Type> concrete = down_pointer_cast<_Type>(message);
		assert(concrete);
		protobufMessageCallback_(conn, concrete);
	}
private:
	ProtobufMessageCallback protobufMessageCallback_;
};

class Dispatcher
{
public:
	using ProtobufMessageCallback = std::function<void(const TcpConnectionPtr &conn, const MessagePtr&)>;
	Dispatcher(ProtobufMessageCallback cb)
		:defaultCallback_(std::move(cb))
	{}

	template <typename _Type>
	void registerMessageCallback(const typename CallbackT<_Type>::ProtobufMessageCallback &callback)
	{
		std::shared_ptr<CallbackT<_Type>> f = std::make_shared<CallbackT<_Type>>(callback);
		callbacks_[_Type::descriptor] = f;
	}

	void onMessage(const TcpConnectionPtr &conn, const MessagePtr &message)
	{
		auto iter = callbacks_.find(message->GetDescriptor());
		if (iter != callbacks_.end())
		{
			iter->second->onMessage(conn, message);
		}
		else
		{
			defaultCallback_(conn, message);
		}
	}
private:
	std::map<const google::protobuf::Descriptor *, std::shared_ptr<Callback>> callbacks_;
	ProtobufMessageCallback defaultCallback_;
};


}








#endif // !_IOEVENT_DISPATCHER_H

