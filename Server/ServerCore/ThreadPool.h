#pragma once

using namespace std;

class ThreadPool
{

public:
	inline int GetNumOfThreads() {
		return m_threads.size();
	};

	void DoTask();
	void Join();

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
