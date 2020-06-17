//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_SINGLETON_H
#define _IOEVENT_SINGLETON_H

#include <boost/noncopyable.hpp>
#include <mutex>
#include <memory>
#include <cassert>

namespace IOEvent
{
template <typename _Type>
class Singleton : public boost::noncopyable
{
public:
	template <typename ...Args>
	static std::shared_ptr<_Type> instance(Args && ...args)
	{
		std::call_once(flag_, [&]()
		{
			inst_ = std::make_shared<_Type>(std::forward<Args>(args)...);
		});
		assert(inst_ != nullptr);
		return inst_;
	}
protected:
	Singleton() = delete;
	~Singleton() = default;
private:
	static std::shared_ptr<_Type> inst_;
	static std::once_flag flag_;
};

template <typename _Type>
std::shared_ptr<_Type> Singleton<_Type>::inst_ = nullptr;

template <typename _Type>
std::once_flag Singleton<_Type>::flag_;

}


#endif // !_IOEVENT_SINGLETON_H
