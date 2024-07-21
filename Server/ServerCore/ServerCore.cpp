#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>
#include <Windows.h>

int Sum()
{
	int sum = 0;
	for (int i = 0; i < 100000; ++i)
	{
		sum++;
	}
	return sum;
}


int main()
{
	std::future<int> f= std::async(std::launch::async, Sum);

	int sum = f.get();
	std::cout << sum;
}