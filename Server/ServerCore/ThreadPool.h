#pragma once

#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
using namespace std;

class ThreadPool
{

public:
	void DoTask();

	ThreadPool();
	~ThreadPool();

	void EnqueueTask(function<void()>task);

private:
	mutex m_mutex;
	condition_variable m_cv;
	vector<thread> m_threads;
	queue<function<void()>> m_tasks;

	bool m_bStop;
};
