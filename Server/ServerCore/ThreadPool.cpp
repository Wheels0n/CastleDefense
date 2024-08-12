#include "stdafx.h"
#include "ThreadPool.h"
#include "ThreadLocal.h"
#include "LockOrderChecker.h"
void ThreadPool::DoTask()
{
	while (true)
	{
		function<void()> task;
		{
			unique_lock<mutex> lock(m_mutex);
			m_cv.wait(lock, [this] {return !m_tasks.empty() || m_bStop; });
			if (m_bStop && m_tasks.empty())
			{
				return;
			}

			task = move(m_tasks.front());
			m_tasks.pop();
		}
		task();
	}
	
}

void ThreadPool::Join()
{
	{
		unique_lock<mutex> lock(m_mutex);
		m_bStop = true;
	}

	m_cv.notify_all();
	for (int i = 0; i < m_threads.size(); ++i)
	{
		if (m_threads[i].joinable())
		{
			delete LLockOrderChecker;
			m_threads[i].join();
		}
	}
}

ThreadPool::ThreadPool() :m_bStop(false)
{
	int nThreads = thread::hardware_concurrency();
	m_threads.reserve(nThreads);

	for (int i = 0; i < nThreads; ++i)
	{
		auto startFunction = [this](int id)
			{
				LThreadId = id;
				LLockOrderChecker = new LockOrderChecker();
				this->DoTask();
			};

		m_threads.emplace_back(startFunction,i);
	}
}

ThreadPool::~ThreadPool()
{
	Join();
}

void ThreadPool::EnqueueTask(function<void()> task)
{
	{
		unique_lock<mutex> lock(m_mutex);
		m_tasks.emplace(move(task));
	}

	m_cv.notify_one();
}
