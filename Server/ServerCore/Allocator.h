#pragma once

const int PAGE_SIZE = 0x1000;
#include <memory.h>
class BaseAllocator
{

public:
	static void* Alloc(int size);
	static void  Release(void* ptr);
};

class StompAllocator
{

public:
	static void* Alloc(int size);
	static void  Release(void* ptr);
};

class PoolAllocator
{
public:
	static void* Alloc(int size);
	static void  Release(void* ptr);
};

template<typename T>
class StlAllocator
{
public:
	typedef T value_type;

	StlAllocator() {}

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	T* allocate(size_t n)
	{
		const int size = static_cast<int>(n * sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t n)
	{
		PoolAllocator::Release(ptr);
	}
};


template<typename T, typename... Args>
T* xnew(Args&&... args)
{
	T* memory = static_cast<T*>(PoolAllocator::Alloc(sizeof(T)));
	new(memory)T(std::forward<Args>(args)...);
	return memory;
}

template<typename T>
void xdelete(T* pObj)
{
	pObj->~T();
	PoolAllocator::Release(pObj);
}

template<typename T, typename... Args>
std::shared_ptr<T> MakeShared(Args&&... args)
{
	return std::shared_ptr<T>{xnew<T>(std::forward<Args>(args)...), xdelete<T>};
}