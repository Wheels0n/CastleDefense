#pragma once
#include <mutex>
#include <queue>

using namespace std;
template<typename T>
class LockQueue
{
public:
	LockQueue() = default;
	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T val)
	{
		std::lock_guard<mutex> lock(m_mutex);
		m_queue.push(std::move(val));
		m_cv.notify_one();
	}

	//Empty()üũ ���Ŀ� �ٸ� �����尡 ä��� �׸�...
	bool TryPop(T& val)
	{
		std::lock_guard<mutex> lock(m_mutex);
		if (m_queue.empty())
		{
			return false;
		}

		val = std::move(m_queue.front());
		m_queue.pop();

		return true;

	}
	//TryPop ���ѹݺ��� ���� ���� ���� ���� ����

	void WaitPop(T& val)
	{
		unique_lock<mutex> lock(m_mutex);
		m_cv.wait(lock, [this] {return m_queue.empty() == false; });
		val = std::move(m_queue.front());
		m_queue.pop();
	}

private:
	queue<T> m_queue;
	mutex m_mutex;
	condition_variable m_cv;
};