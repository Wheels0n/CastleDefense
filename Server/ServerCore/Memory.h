#pragma once
#include "stdafx.h"
static const int MAX_ALLOC_SIZE = 4096;
static const int MEM_POOL_CNT = (1024 / 32) + (1024 / 128) + (2048 / 256);

struct MemoryHeader : SLIST_ENTRY
{	
	MemoryHeader(int size) : m_allocSize(size) {};
	long m_allocSize;
};

inline void*			AttachMemoryHeader(MemoryHeader* pHeader, int size)
{
	new(pHeader)MemoryHeader(size);
	return reinterpret_cast<void*>(++pHeader);
}
inline MemoryHeader*	DetachMemoryHeader(void* ptr)
{
	return reinterpret_cast<MemoryHeader*>(ptr) - 1;
}

class MemoryPool
{
public:
	int				GetNumAlloc()
	{
		return m_nAlloc.load();
	}

	void			Push(MemoryHeader* pHeader);
	MemoryHeader*	Pop();

					MemoryPool(int allocSize);
					~MemoryPool();
private:
	SLIST_HEADER	m_pHeader;
	int				m_allocSize;
	atomic<int>		m_nAlloc;
};


class MemoryPoolManager
{
private:
			MemoryPoolManager();
			MemoryPoolManager(const MemoryPoolManager& obj) = delete;
public:
	void*	Allocate(int size);
	void	Deallocate(void* ptr);

	static MemoryPoolManager& GetInstance()
	{
		static MemoryPoolManager instance;
		return instance;
	}
private:
	MemoryPool* m_memoryPoolTable[MAX_ALLOC_SIZE+1];
};


template<typename T>
class ObjectPool
{
public:
	template<typename... Args>
	static T*				Pop(Args&&... args)
	{
		T* pMem = static_cast<T*>(AttachMemoryHeader(m_memoryPool.Pop(), m_allocSize));
		new(pMem)T(std::forward<Args>(args)...);
		return pMem;
	}
	static void				Push(T* pObj)
	{
		pObj->~T();
		m_memoryPool.Push(DetachMemoryHeader(pObj));
	}

	static shared_ptr<T>	MakeShared()
	{
		return std::shared_ptr<T> {Pop(), Push()};
	}
	static int				GetNumOfAlloc()
	{
		return m_memoryPool.GetNumAlloc();
	}
private:
	static int			m_allocSize;
	static MemoryPool	m_memoryPool;
};

template<typename T>
int ObjectPool<T>::m_allocSize = sizeof(T) + sizeof(MemoryPool);

template<typename T>
MemoryPool ObjectPool<T>::m_memoryPool{ m_allocSize };
