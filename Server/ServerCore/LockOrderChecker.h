#pragma once
#include <stack>

class RWLock;
class LockOrderChecker
{
private:
	LockOrderChecker()=default;
	LockOrderChecker(const LockOrderChecker& obj) = delete;

public:
	void Push(RWLock* pLock);
	void Pop(RWLock* pLock);

	static LockOrderChecker& GetInstance()
	{
		thread_local LockOrderChecker instance;
		return instance;
	}
private:
	stack<RWLock*> m_lockStack;
};

