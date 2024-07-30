#pragma once

#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>
#include <Windows.h>

#include "ThreadPool.h"
#include "Lock.h"

RWLock a(100);
RWLock b(1000);
void AToB()
{
	cout << "AtoB" << endl;
	WriteLockGuard aGuard(a);
	WriteLockGuard bGuard(b);
}

void BToA()
{
	cout << "BtoA" << endl;
	WriteLockGuard bGuard(b);
	WriteLockGuard aGuard(a);
}


int main()
{
	ThreadPool threadPool;

	while(true)
	{
		threadPool.EnqueueTask(AToB);
		threadPool.EnqueueTask(AToB);
	}
}