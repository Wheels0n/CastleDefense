#include "LockOrderChecker.h"
#include "Lock.h"
void LockOrderChecker::Push(RWLock* pLock)
{
	if (!m_lockStack.empty())
	{
		RWLock* pPrevLock = m_lockStack.top();
		if (pPrevLock->GetOrder() > pLock->GetOrder())
		{
			//Å©·¡½Ã
			int* ptr = nullptr;
			*ptr = 0xDEAD;
		}
	}
	m_lockStack.push(pLock);
}

void LockOrderChecker::Pop(RWLock* pLock)
{
	if (!m_lockStack.empty())
	{
		m_lockStack.pop();
	}
}
