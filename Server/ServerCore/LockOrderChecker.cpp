#include "stdafx.h"
#include "LockOrderChecker.h"
#include "Lock.h"
void LockOrderChecker::Push(RWLock* pLock)
{
	if (!m_lockStack.empty())
	{
		RWLock* pPrevLock = m_lockStack.top();
		assert(pPrevLock->GetOrder() <= pLock->GetOrder());
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
