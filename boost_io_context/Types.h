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

template<typename To, typename From>
inline std::shared_ptr<To> down_pointer_cast(const std::shared_ptr<From>& f) 
{
	assert(f == nullptr || dynamic_cast<To*>(get_pointer(f)) != nullptr);
	return std::static_pointer_cast<To>(f);
}

}


#endif // !_IOEVENT_TYPES_H
