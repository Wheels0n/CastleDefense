#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
	void lock()
	{	
		bool expected = false;
		bool desired = true;
		while (m_bLocked.compare_exchange_strong(expected, desired)==false)
		{
			expected = false;
		}
	}
	void unlock()
	{
		m_bLocked = false;
	}

private:
	std::atomic<bool> m_bLocked=false;
};

SpinLock g_lock;
int n = 0;
void Add()
{
	for (int i = 0; i < 100000; ++i)
	{
		std::lock_guard<SpinLock> spinLock(g_lock);
		n++;
	}
}
void Sub()
{	
	for (int i = 0; i < 100000; ++i)
	{	
		std::lock_guard<SpinLock> spinLock(g_lock);
		n--;
	}
}


int main()
{
	std::thread t1(Add);
	std::thread t2(Sub);
	t1.join();
	t2.join();

	std::cout << n;
}