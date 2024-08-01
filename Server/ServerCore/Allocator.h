#pragma once
class BaseAllocator
{

public:
	static void* Alloc(int size);
	static void  Release(void* ptr);
};

