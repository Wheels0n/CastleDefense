#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <Windows.h>

std::mutex m;
int n = 0;
std::condition_variable cv;
void Producer()
{
	while(true)
	{
		if (n)
		{
			std::this_thread::yield();
		}
		else
		{
			{
				std::unique_lock<std::mutex> ulock(m);
				n++;
			}

			cv.notify_one();
		}
		
		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
void Consumer()
{	
	while (true)
	{	
		std::unique_lock<std::mutex> ulock(m);
		cv.wait(ulock, []() {return n > 0; });
		--n;
		std::cout << n << std::endl;
		
		
	}
}


int main()
{
	std::thread t2(Consumer);
	std::thread t1(Producer);
	t1.join();
	t2.join();

	std::cout << n;
}