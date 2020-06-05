﻿#ifndef _IOEVENT_CALLBACK_H
#define _IOEVENT_CALLBACK_H

#include <memory>
#include <functional>

namespace IOEvent
{
class TcpConnection;
class Buffer;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
typedef std::function<void(const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void(const TcpConnectionPtr&)> CloseCallback;
typedef std::function<void(const TcpConnectionPtr&)> WriteCompleteCallback;
typedef std::function<void(const TcpConnectionPtr&, Buffer *)> MessageCallback;
}



#endif // !_IOEVENT_CALLBACK_H
