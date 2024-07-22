#pragma once

#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>
#include <Windows.h>

#include "LockStack.h"
#include "LockQueue.h"

LockStack<int> g_stack;
LockQueue<int> g_queue;

void Producer()
{
	while (true)
	{
		int val = rand()&100;
		g_queue.Push(val);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void Consumer()
{
	while (true)
	{
		int val = 0;
		if (g_queue.TryPop(val))
		{
			std::cout << val << std::endl;
		}
	}
}




int main()
{
	std::thread t1(Producer);
	std::thread t2(Consumer);

	t1.join();
	t2.join();
}