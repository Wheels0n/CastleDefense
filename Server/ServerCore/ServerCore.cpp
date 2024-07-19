#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <vector>
std::mutex g_m1;
std::mutex g_m2;
void Lock1()
{
	for (int i = 0; i < 1; ++i)
	{
		std::lock_guard<std::mutex> lockGuard1(g_m1);
		std::lock_guard<std::mutex> lockGuard2(g_m2);
	}
}
void Lock2()
{	
	for (int i = 0; i < 1; ++i)
	{	
		std::lock_guard<std::mutex> lockGuard2(g_m2);
		std::lock_guard<std::mutex> lockGuard1(g_m1);
	}
}


int main()
{
	std::thread t1(Lock1);
	std::thread t2(Lock2);
	t1.join();
	t2.join();

}