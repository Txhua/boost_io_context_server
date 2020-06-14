//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_TIMER_H
#define _IOEVENT_TIMER_H

#include <boost/noncopyable.hpp>
#include <functional>
#include <atomic>
#include "Timestamp.h"


namespace IOEvent
{

class Timer final : public boost::noncopyable
{
	using TimerCallback = std::function<void()>;
public:
	Timer(TimerCallback cb, const Timestamp &expiration, double interval);	
	bool repeat()const { return repeat_; }
	const Timestamp &expiration()const { return expiration_; }
	int64_t sequence() const { return sequence_; }
	void run() { callback_(); }
	void restart(const Timestamp &now);
	static int64_t numCreated() { return s_numCreated_; }
private:
	Timestamp expiration_;
	double interval_;
	bool repeat_;
	TimerCallback callback_;
	int64_t sequence_;
	static std::atomic<int64_t> s_numCreated_;
};

}





#endif // !_IOEVENT_TIMER_H

