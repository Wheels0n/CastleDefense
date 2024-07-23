#pragma once

#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>
#include <Windows.h>

#include "ThreadPool.h"


int main()
{
	ThreadPool threadPool;

	while (true)	
	{
		threadPool.EnqueueTask([] {
			cout << "Doing Task" << " " << this_thread::get_id() << endl;
			this_thread::sleep_for(chrono::milliseconds(100));
			});
	}
	
}