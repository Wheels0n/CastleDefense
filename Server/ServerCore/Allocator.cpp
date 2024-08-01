#include "Allocator.h"
#include <cstdlib>
#include <windows.h>
#include <memoryapi.h>
void* BaseAllocator::Alloc(int size)
{
    return malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
    free(ptr);
}

void* StompAllocator::Alloc(int size)
{
    const int nPage = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    void* pBase = VirtualAlloc(NULL, nPage * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return pBase;
}

void StompAllocator::Release(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}
