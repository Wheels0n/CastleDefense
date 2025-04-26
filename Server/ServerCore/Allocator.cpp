#include "stdafx.h"
#include "Allocator.h"
#include "Memory.h"

void*   BaseAllocator::Alloc(int size)
{
    return malloc(size);
}
void    BaseAllocator::Release(void* ptr)
{
    free(ptr);
}

void*   StompAllocator::Alloc(int size)
{
    const int nPage = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    void* pBase = VirtualAlloc(NULL, nPage * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return pBase;
}
void    StompAllocator::Release(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

void*   PoolAllocator::Alloc(int size)
{
    return MemoryPoolManager::GetInstance().Allocate(size);
}
void    PoolAllocator::Release(void* ptr)
{
    MemoryPoolManager::GetInstance().Deallocate(ptr);
}
