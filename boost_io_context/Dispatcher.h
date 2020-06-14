//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_DISPATCHER_H
#define _IOEVENT_DISPATCHER_H

#include <boost/noncopyable.hpp>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <map>
#include "Callbacks.h"
#include "Types.h"

namespace IOEvent
{
using MessagePtr = std::shared_ptr<google::protobuf::Message> ;

class Callback
{
public:
	virtual ~Callback() = default;
	virtual void onMessage(const TcpConnectionPtr&,const MessagePtr& message) const = 0;
};

template <typename T>
class CallbackT : public Callback
{
	static_assert(std::is_base_of<google::protobuf::Message, T>::value,"T must be derived from gpb::Message.");
public:
	using ProtobufMessageTCallback = std::function<void(const TcpConnectionPtr&,const std::shared_ptr<T>& message)>;

	CallbackT(const ProtobufMessageTCallback& callback)
		: callback_(callback)
	{
	}
	void onMessage(const TcpConnectionPtr& conn,const MessagePtr& message) const override
	{
		std::shared_ptr<T> concrete = down_pointer_cast<T>(message);
		assert(concrete != nullptr);
		callback_(conn, concrete);
	}

private:
	ProtobufMessageTCallback callback_;
};

class Dispatcher
{
public:
	using ProtobufMessageCallback = std::function<void(const TcpConnectionPtr&,const MessagePtr& message)>;

	explicit Dispatcher(const ProtobufMessageCallback& defaultCb)
		: defaultCallback_(defaultCb)
	{
	}

	void onMessage(const TcpConnectionPtr& conn,const MessagePtr& message) const
	{
		CallbackMap::const_iterator it = callbacks_.find(message->GetDescriptor());
		if (it != callbacks_.end())
		{
			it->second->onMessage(conn, message);
		}
		else
		{
			defaultCallback_(conn, message);
		}
	}

	template<typename T>
	void registerMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& callback)
	{
		std::shared_ptr<CallbackT<T> > pd(new CallbackT<T>(callback));
		callbacks_[T::descriptor()] = pd;
	}

private:
	using CallbackMap = std::map<const google::protobuf::Descriptor*, std::shared_ptr<Callback>>;

	CallbackMap callbacks_;
	ProtobufMessageCallback defaultCallback_;
};
}


#endif // !_IOEVENT_DISPATCHER_H

