#include "Allocator.h"
#include <cstdlib>
void* BaseAllocator::Alloc(int size)
{
    return malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
    free(ptr);
}
