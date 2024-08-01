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