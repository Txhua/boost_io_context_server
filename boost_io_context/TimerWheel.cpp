#include "TimerWheel.h"

namespace IOEvent
{
std::atomic<uint32_t> TimerEvent::s_numCreated_ = 0;

TimerWheel::TimerWheel(uint32_t slotsNum, bool isThreadSave)
	:slotsNum_(slotsNum),
	ts_(1),
	slots_(slotsNum_),
	mutex_(isThreadSave ? std::make_unique<std::mutex>() : nullptr)
{
	for (uint32_t i = 0; i < slotsNum_; ++i)
	{
		slots_[i] = nullptr;
	}
}
TimerWheel::~TimerWheel()
{
	// 释放时间轮
	for (uint32_t i = 0; i < slotsNum_; ++i)
	{
		auto *tmp = slots_[i];
		while (tmp)
		{
			slots_[i] = tmp->pNext_;
			delete tmp;
			tmp = slots_[i];
		}
	}
}
CancelId TimerWheel::addTimer(const uint32_t timeout, const std::shared_ptr<void> &obj, TimerMessageCallback cb)
{
	if (mutex_)
	{
		std::unique_lock<std::mutex> lock(*mutex_);
		return addTimerUnlock(timeout, obj, std::move(cb));
	}
	else
	{
		return addTimerUnlock(timeout, obj, std::move(cb));
	}
}

CancelId TimerWheel::addTimerUnlock(const uint32_t timeout, const std::shared_ptr<void>& obj, TimerMessageCallback cb)
{
	auto tick = 0;
	if (timeout < ts_)
	{
		tick = 1;
	}
	else
	{
		tick = timeout / ts_;
	}

	// 先确定在时间轮的位置
	int rotation = tick / slotsNum_;
	auto pos = (currentSlotPos_ + (tick % slotsNum_)) % slotsNum_;
	auto *timerEvent = new TimerEvent(rotation, pos, std::move(cb), obj);
	if (!slots_[pos])
	{
		slots_[pos] = timerEvent;
	}
	else
	{
		auto *curSlot = slots_[pos];
		timerEvent->pNext_ = curSlot;
		curSlot->pPrev_ = timerEvent;
		slots_[pos] = timerEvent;
	}
	return CancelId(timerEvent->sequence(), timerEvent);
}

void TimerWheel::cencel(CancelId timeId)
{
	if (mutex_)
	{
		std::unique_lock<std::mutex> lock(*mutex_);
		cencelUnlock(timeId);
	}
	else
	{
		cencelUnlock(timeId);
	}
}

void TimerWheel::cencelUnlock(CancelId timeId)
{
	if (!timeId.timer_)
	{
		return;
	}
	auto pos = timeId.timer_->timerSlot();
	if (timeId.timer_ == slots_[pos])
	{
		auto *nextEvent = timeId.timer_->pNext_;
		slots_[pos] = nextEvent;
		if (nextEvent)
		{
			nextEvent->pPrev_ = nullptr;
		}
		delete timeId.timer_;
	}
	else
	{
		timeId.timer_->pPrev_->pNext_ = timeId.timer_->pNext_;
		if (timeId.timer_->pNext_)
		{
			timeId.timer_->pNext_->pPrev_ = timeId.timer_->pPrev_;
		}
		delete timeId.timer_;
	}
}


void TimerWheel::tick()
{
	if (mutex_)
	{
		std::unique_lock<std::mutex> lock(*mutex_);
		tickUnlock();
	}
	else
	{
		tickUnlock();
	}
}

void TimerWheel::tickUnlock()
{
	auto *timerEvent = slots_[currentSlotPos_];
	while (timerEvent)
	{
		int rotation = timerEvent->rotation();
		if (rotation > 0)
		{
			timerEvent->setRotation(rotation - 1);
			timerEvent = timerEvent->pNext_;
		}
		else
		{
			// 执行超时回调
			timerEvent->run();
			// 链表头
			if (timerEvent == slots_[currentSlotPos_])
			{
				auto nextEvent = timerEvent->pNext_;
				slots_[currentSlotPos_] = nextEvent;
				delete timerEvent;
				if (slots_[currentSlotPos_])
				{
					slots_[currentSlotPos_]->pPrev_ = nullptr;
				}
				timerEvent = slots_[currentSlotPos_];
			}
			else
			{
				timerEvent->pPrev_->pNext_ = timerEvent->pNext_;
				if (timerEvent->pNext_)
				{
					timerEvent->pNext_->pPrev_ = timerEvent->pPrev_;
				}
				auto *tmp = timerEvent->pNext_;
				delete timerEvent;
				timerEvent = tmp;
			}
		}
	}
	currentSlotPos_ = ++currentSlotPos_ % slotsNum_;
}


}
