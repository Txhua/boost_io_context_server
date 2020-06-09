#include "Timestamp.h"

namespace IOEvent
{
Timestamp::Timestamp(const std::chrono::microseconds & micr)
	:microSecondsSinceEpoch_(micr)
{}
Timestamp::Timestamp()
	: microSecondsSinceEpoch_(0)
{}
void Timestamp::swap(Timestamp & rhs)
{
	std::swap(microSecondsSinceEpoch_, rhs.microSecondsSinceEpoch_);
}
Timestamp Timestamp::now()
{
	auto now = std::chrono::system_clock::now();
	return Timestamp(std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()));
}
Timestamp Timestamp::invalid()
{
	return Timestamp();
}
}
