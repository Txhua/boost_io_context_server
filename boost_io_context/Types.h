#ifndef _IOEVENT_TYPES_H
#define _IOEVENT_TYPES_H

#include <memory>

namespace IOEvent
{
template<typename _Type>
inline _Type* get_pointer(const std::shared_ptr<_Type>& ptr)
{
	return ptr.get();
}

template<typename _Type>
inline _Type* get_pointer(const std::unique_ptr<_Type>& ptr)
{
	return ptr.get();
}
}


#endif // !_IOEVENT_TYPES_H
