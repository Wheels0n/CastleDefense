#pragma once
#include "stdafx.h"
const int MAX_ALLOC_SIZE = 4096;
const int MEM_POOL_CNT = (1024 / 32) + (1024 / 128) + (2048 / 256);

struct MemoryHeader : SLIST_ENTRY
{
		
	MemoryHeader(int size) : m_allocSize(size) {};

	long m_allocSize;
};

inline void* AttachMemoryHeader(MemoryHeader* pHeader, int size)
{
	new(pHeader)MemoryHeader(size);
	return reinterpret_cast<void*>(++pHeader);
}

inline MemoryHeader* DetachMemoryHeader(void* ptr)
{
	return reinterpret_cast<MemoryHeader*>(ptr) - 1;
}


class MemoryPool
{
public:
	MemoryPool(int allocSize);
	~MemoryPool();

	int GetNumAlloc()
	{
		return m_nAlloc.load();
	}
	void Push(MemoryHeader* pHeader);
	MemoryHeader* Pop();
private:
	SLIST_HEADER m_pHeader;
	int m_allocSize;
	std::atomic<int> m_nAlloc;
};

class MemoryPoolManager
{
public:
	MemoryPoolManager();

	void* Allocate(int size);
	void Deallocate(void* ptr);
private:
	MemoryPool* m_memoryPoolTable[MAX_ALLOC_SIZE+1];
};

extern MemoryPoolManager* g_pMemoryPoolManager;

template<typename T>
class ObjectPool
{
public:
	template<typename... Args>
	static T* Pop(Args&&... args)
	{
		T* pMem = static_cast<T*>(AttachMemoryHeader(m_memoryPool.Pop(), m_allocSize));
		new(pMem)T(std::forward<Args>(args)...);
		return pMem;
	}

	static void Push(T* pObj)
	{
		pObj->~T();
		m_memoryPool.Push(DetachMemoryHeader(pObj));
	}

	static std::shared_ptr<T> MakeShared()
	{
		return std::shared_ptr<T> {Pop(), Push()};
	}

	static int GetNumOfAlloc()
	{
		return m_memoryPool.GetNumAlloc();
	}
private:
	static int m_allocSize;
	static MemoryPool m_memoryPool;
};

template<typename T>
int ObjectPool<T>::m_allocSize = sizeof(T) + sizeof(MemoryPool);

template<typename T>
MemoryPool ObjectPool<T>::m_memoryPool{ m_allocSize };
