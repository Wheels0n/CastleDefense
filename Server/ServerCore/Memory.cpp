#include "stdafx.h"
#include "Memory.h"


MemoryPoolManager* g_pMemoryPoolManager = nullptr;

MemoryPool::MemoryPool(int allocSize) : m_allocSize(allocSize), m_nAlloc(0)
{
	InitializeSListHead(&m_pHeader);
}

MemoryPool::~MemoryPool()
{
	while (MemoryHeader* mem = static_cast<MemoryHeader*>(InterlockedPopEntrySList(&m_pHeader)))
	{
		_aligned_free(mem);
	}
}

void MemoryPool::Push(MemoryHeader* pHeader)
{
	pHeader->m_allocSize = 0;
	InterlockedPushEntrySList(&m_pHeader, pHeader);
	m_nAlloc.fetch_sub(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* mem =  static_cast<MemoryHeader*>(InterlockedPopEntrySList(&m_pHeader));
	if (mem == nullptr)
	{
		mem = reinterpret_cast<MemoryHeader*>(_aligned_malloc(m_allocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		assert(mem->m_allocSize == 0);
	}

	m_nAlloc.fetch_add(1);
	return mem;
}

MemoryPoolManager::MemoryPoolManager()
{
	memset(m_memoryPoolTable, 0, sizeof(m_memoryPoolTable));

	int last = 0;

	for (int i = 32; i < 1024; i += 32)
	{
		MemoryPool* pMemoryPool = new MemoryPool(i);
		for (int j = last + 1; j <= i; ++j)
		{
			m_memoryPoolTable[j] = pMemoryPool;
		}
		
		last = i;
	}

	for (int i =1024; i < 2048; i += 128)
	{
		MemoryPool* pMemoryPool = new MemoryPool(i);
		for (int j = last + 1; j <= i; ++j)
		{
			m_memoryPoolTable[j] = pMemoryPool;
		}

		last = i;
	}

	for (int i = 2048; i < 4096; i += 256)
	{
		MemoryPool* pMemoryPool = new MemoryPool(i);
		for (int j = last + 1; j <= i; ++j)
		{
			m_memoryPoolTable[j] = pMemoryPool;
		}

		last = i;
	}
}

void* MemoryPoolManager::Allocate(int size)
{
	MemoryHeader* pMemoryHeader = nullptr;
	int realAllocSize = size + sizeof(MemoryHeader);

	//4096보다 크면 그냥 할당
	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		pMemoryHeader = reinterpret_cast<MemoryHeader*>(_aligned_malloc(realAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		MemoryPool* pMemoryPool = m_memoryPoolTable[realAllocSize];
		pMemoryHeader = pMemoryPool->Pop();
	}

	return AttachMemoryHeader(pMemoryHeader, realAllocSize);
}

void MemoryPoolManager::Deallocate(void* ptr)
{
	MemoryHeader* pHeader = DetachMemoryHeader(ptr);
	
	long realAllocSize = pHeader->m_allocSize;
	assert(realAllocSize > 0);
	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		_aligned_free(pHeader);
	}
	else
	{
		MemoryPool* pMemoryPool = m_memoryPoolTable[realAllocSize];
		pMemoryPool->Push(pHeader);
	}
}
