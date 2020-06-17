//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_TIMER_WHEEL_H
#define _IOEVENT_TIMER_WHEEL_H

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include <boost/circular_buffer.hpp>
#include <boost/noncopyable.hpp>

namespace IOEvent
{
using TimerMessageCallback = std::function<void()>;

class TimerEvent final
{
public:
	explicit TimerEvent(int rotation, int timerSlot, TimerMessageCallback callback, const std::shared_ptr<void>&obj)
		:rotation_(rotation),
		timerSlot_(timerSlot),
		callback_(std::move(callback)),
		tie_(obj),
		tied_(obj ? true : false),
		sequence_(++s_numCreated_),
		pPrev_(nullptr),
		pNext_(nullptr)
	{}

	uint32_t sequence() const { return sequence_; }
	void run() const
	{
		if (callback_)
		{
			if (tied_)
			{
				auto guard = tie_.lock();
				if (guard)
				{
					callback_();
				}
			}
			else
			{
				callback_();
			}
		}
	}
	int timerSlot()const { return timerSlot_; }
	int rotation()const { return rotation_; }
	void setRotation(int rotation) { rotation_ = rotation; }
	class TimerEvent *pPrev_;
	class TimerEvent *pNext_;
	static uint32_t numCreated() { return s_numCreated_; }
private:
	// 时间轮转动的圈数
	int rotation_;
	// 当前分配的槽
	int timerSlot_;
	// 超时回调函数(只认std::bind())
	TimerMessageCallback callback_;
	// 是否需要保活
	bool tied_;
	// 用户数据(保活)
	std::weak_ptr<void> tie_;
	// 分配timerid
	static std::atomic<uint32_t> s_numCreated_;
	const uint32_t sequence_;
};

class CancelId final
{
public:
	explicit CancelId(const int sequence, TimerEvent *timer)
		:sequence_(sequence),
		timer_(timer)
	{}
	CancelId()
		:sequence_(0),
		timer_(nullptr)
	{}
private:
	friend class TimerWheel;
	const int sequence_;
	TimerEvent *timer_;
};

class TimerWheel final : public boost::noncopyable
{
public:
	TimerWheel(uint32_t slotsNum = 512, bool isThreadSave = true);
	~TimerWheel();
	CancelId addTimer(const uint32_t timeout, const std::shared_ptr<void>&obj, TimerMessageCallback cb);
	void cencel(CancelId timeId);
	void tick();
private:
	CancelId addTimerUnlock(const uint32_t timeout, const std::shared_ptr<void>&obj, TimerMessageCallback cb);
	void cencelUnlock(CancelId timeId);
	void tickUnlock();
private:
	// 时间轮的槽个数
	const uint32_t slotsNum_;
	// 时间间隔
	const uint32_t ts_;
	// 时间轮当前的位置
	int currentSlotPos_;
	// 安全锁
	std::unique_ptr<std::mutex> mutex_;
	// 环形时间轮
	boost::circular_buffer<TimerEvent *> slots_;
};


}

#endif // !_IOEVENT_TIMER_WHEEL_H
