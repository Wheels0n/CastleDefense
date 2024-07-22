#pragma once
#include <stack>
#include <mutex>

using namespace std;
template<typename T>
class LockStack
{
public:
	LockStack() = default;
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T val)
	{
		std::lock_guard<mutex> lock(m_mutex);
		m_stack.push(std::move(val));
		m_cv.notify_one();
	}

	//Empty()üũ ���Ŀ� �ٸ� �����尡 ä��� �׸�...
	bool TryPop(T& val)
	{
		std::lock_guard<mutex> lock(m_mutex);
		if (m_stack.empty())
		{
			return false;
		}

		val = std::move(m_stack.top());
		m_stack.pop();

		return true;

	}
	//TryPop ���ѹݺ��� ���� ���� ���� ���� ����

	void WaitPop(T& val)
	{
		unique_lock<mutex> lock(m_mutex);
		m_cv.wait(lock, [this] {return m_stack.empty() == false; });
		val = std::move(m_stack.top());
		m_stack.pop();
	}

private:
	stack<T> m_stack;
	mutex m_mutex; 
	condition_variable m_cv;
};