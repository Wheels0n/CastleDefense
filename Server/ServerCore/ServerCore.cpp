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
class TestLock
{
public:
	int TestRead()
	{
		int val = -1;
		m_lock.ReadLock();
		if (!m_q.empty())
		{
			val= m_q.front();
		}
		m_lock.ReadUnlock();

		return val;
	};
	void TestPush()
	{
		m_lock.WriteLock();
		m_q.push(rand() % 100);
		m_lock.WriteUnlock();
	}
	void TestPop()
	{
		m_lock.WriteLock();
		if (!m_q.empty())
		{
			m_q.pop();
		}
		m_lock.WriteUnlock();
	}
private:
	queue<int> m_q;
	Lock m_lock;
};

TestLock g_testLock;
void ThreadWrite()
{
	{
		g_testLock.TestPush();
		this_thread::sleep_for(chrono::milliseconds(1ms));
		g_testLock.TestPop();
	}
}

void ThreadRead()
{
	
	{
		int val = g_testLock.TestRead();
		cout << val << endl;
		this_thread::sleep_for(chrono::milliseconds(1ms));
	}
}

int main()
{
	ThreadPool threadPool;

	while(true)
	{
		threadPool.EnqueueTask(ThreadWrite);
		threadPool.EnqueueTask(ThreadWrite);
		threadPool.EnqueueTask(ThreadWrite);
		threadPool.EnqueueTask(ThreadRead);
		threadPool.EnqueueTask(ThreadRead);
		threadPool.EnqueueTask(ThreadRead);
	}
}