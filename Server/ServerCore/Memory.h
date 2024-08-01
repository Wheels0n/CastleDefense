#pragma once
#include "Allocator.h"
#include <utility>
template<typename T, typename... Args>
T* xnew(Args&&... args)
{
	T* memory = static_cast<T*>(StompAllocator::Alloc(sizeof(T)));
	new(memory)T(std::forward<Args>(args)...);
	return memory;
}

template<typename T>
void xdelete(T* pObj)
{
	pObj->~T();
	StompAllocator::Release(pObj);
}
