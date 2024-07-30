#pragma once
#include <stack>
class RWLock;

class LockOrderChecker
{
public:
	
	void Push(RWLock* pLock);
	void Pop(RWLock* pLock);
private:
	std::stack<RWLock*> m_lockStack;
};

