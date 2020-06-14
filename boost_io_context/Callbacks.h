//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.


#ifndef _IOEVENT_CALLBACK_H
#define _IOEVENT_CALLBACK_H

#include <memory>
#include <functional>

namespace IOEvent
{
class TcpConnection;
class Buffer;
using TimerCallback = std::function<void()>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer *)>;
}



#endif // !_IOEVENT_CALLBACK_H
