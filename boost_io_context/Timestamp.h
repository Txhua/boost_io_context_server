//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_TIMESTAMP_H
#define _IOEVENT_TIMESTAMP_H

#include <boost/operators.hpp>
#include <chrono>

namespace IOEvent
{

class Timestamp final :
	public boost::less_than_comparable<Timestamp>,
	public boost::equality_comparable<Timestamp>
{
public:
	explicit Timestamp(const std::chrono::microseconds &micr);	
	Timestamp();	
	void swap(Timestamp &rhs);
	bool vaild()const { return microSecondsSinceEpoch_.count() > 0; }
	static Timestamp now();	
	static Timestamp invalid();	
	int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_.count(); }
	static const int kMicroSecondsPerSecond = 1000 * 1000;
private:
	std::chrono::microseconds microSecondsSinceEpoch_;
};

inline bool operator < (const Timestamp &lhs, const Timestamp &rhs)
{
	return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator == (const Timestamp &lhs, const Timestamp &rhs)
{
	return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
	int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
	return Timestamp(std::chrono::microseconds(timestamp.microSecondsSinceEpoch() + delta));
}

}



#endif // !_IOEVENT_TIMESTAMP_H

