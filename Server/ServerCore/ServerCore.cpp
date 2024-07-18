#include <thread>
#include <iostream>
#include <atomic>
std::atomic<int> g_val=0;
void Increase()
{
	for (int i = 0; i < 50; ++i)
	{
		g_val++;
	}
}
void Decrease()
{
	for (int i = 0; i < 50; ++i)
	{
		g_val--;
	}
}
int main()
{
	std::thread t1(Increase);
	std::thread t2(Decrease);
	t1.join();
	t2.join();

	std::cout << g_val;
}