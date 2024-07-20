#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <Windows.h>

std::mutex m;
HANDLE g_handle;
int n = 0;
void Add()
{
	for (int i = 0; i < 100; ++i)
	{
		{
			std::lock_guard<std::mutex> lockGuard(m);
			n++;
		}
		
		SetEvent(g_handle);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
void Sub()
{	
	for (int i = 0; i < 100; ++i)
	{	
		WaitForSingleObject(g_handle, INFINITE);
		std::lock_guard<std::mutex> lockGuard(m);
		n--;
	}
}


int main()
{
	g_handle = CreateEvent(NULL, FALSE, FALSE, NULL);
	std::thread t1(Add);
	std::thread t2(Sub);
	t1.join();
	t2.join();

	std::cout << n;
}