#include "ThreadPool.h"

void ThreadPool::DoTask()
{
	while (true)
	{
		unique_lock<mutex> lock(m_mutex);

		m_cv.wait(lock, [this] {return !m_tasks.empty() || m_bStop; });

		if (m_bStop && m_tasks.empty())
		{
			return;
		}

		function<void()> task;
		task = move(m_tasks.front());
		m_tasks.pop();
		task();
	}
	
}

ThreadPool::ThreadPool() :m_bStop(false)
{
	int nThreads = thread::hardware_concurrency();
	m_threads.reserve(nThreads);

	for (int i = 0; i < nThreads; ++i)
	{
		m_threads.emplace_back([this]() {this->DoTask(); });
	}
}

ThreadPool::~ThreadPool()
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
			m_threads[i].join();
		}
	}
}

void ThreadPool::EnqueueTask(function<void()> task)
{
	{
		unique_lock<mutex> lock(m_mutex);
		m_tasks.emplace(move(task));
	}

	m_cv.notify_one();
}
