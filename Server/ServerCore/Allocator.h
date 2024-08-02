#pragma once

const int PAGE_SIZE = 0x1000;
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
		return static_cast<T*>(StompAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t n)
	{
		StompAllocator::Release(ptr);
	}
};