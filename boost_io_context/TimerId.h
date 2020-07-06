//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_TIMERID_H
#define _IOEVENT_TIMERID_H

namespace IOEvent
{
class Timer;
class TimerId final
{
public:
	explicit TimerId(Timer *timer, int64_t sequence)
		:timer_(timer),
		sequence_(sequence)
	{}

	TimerId()
		:timer_(nullptr),
		sequence_(0)
	{}

private:
	friend class TimerQueue;
	Timer *timer_;
	int64_t sequence_;
};


}









#endif // !_IOEVENT_TIMERID_H
