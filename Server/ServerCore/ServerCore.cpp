#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <vector>
std::mutex g_m;
std::vector<int> g_vec;
void Push()
{
	for (int i = 0; i < 50; ++i)
	{
		std::lock_guard<std::mutex> lockGuard(g_m);
		g_vec.push_back(i);
	}
}

int main()
{
	std::thread t1(Push);
	std::thread t2(Push);
	t1.join();
	t2.join();

	std::cout << g_vec.size();
}