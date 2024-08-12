#pragma once
#include "stdafx.h"
#include "ThreadPool.h"
#include "Lock.h"

#include "Memory.h"


class A
{
public:
	A() { cout << "A Constructor" << endl; };
	~A() { cout << "A Destructor" << endl; };

};
class B : public A
{
public:
	B() { cout << "B Constructor" << endl; };
	~B() { cout << "B Destructor" << endl; };
	int num;
};


int main()
{
	g_pMemoryPoolManager = new MemoryPoolManager();
	A* As[100];
	for (int i = 0; i < 100; ++i)
	{
		As[i] = ObjectPool<A>::Pop();
	}

	for (int i = 0; i < 100; ++i)
	{
		ObjectPool<A>::Push(As[i]);
	}
	shared_ptr<A> pA = MakeShared<A>();
	ThreadPool* pThreadPool = new ThreadPool();
/*
	while (true)
	{
		pThreadPool->EnqueueTask([]()
			{
				A* pA = xnew<A>();
				this_thread::sleep_for(std::chrono::microseconds(10));
				xdelete<A>(pA);
			});
		this_thread::sleep_for(std::chrono::microseconds(100));
	}
*/
}