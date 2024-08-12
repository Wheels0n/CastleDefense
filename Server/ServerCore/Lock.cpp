#include "stdafx.h"
#include "Lock.h"
#include "LockOrderChecker.h"
#include "ThreadLocal.h"
void RWLock::WriteLock()
{
	// r/w �� �� ����� ��
	LLockOrderChecker->Push(this);
	uint32_t expected = eLockFlag::EMPTY;
	uint32_t desired = eLockFlag::ON_WRITE;
	while (m_lockFlag.compare_exchange_strong(expected, desired)==false)
	{
		expected = eLockFlag::EMPTY;
	}

}

void RWLock::WriteUnlock()
{
	LLockOrderChecker->Pop(this);
	m_lockFlag.store(eLockFlag::EMPTY);
}

void RWLock::ReadLock()
{
	//��ģ�� ���� r/w�� 
	LLockOrderChecker->Push(this);
	uint32_t expected = m_lockFlag.load()&eLockFlag::ON_READ;
	uint32_t desired = expected+1;
	while (m_lockFlag.compare_exchange_strong(expected, desired)==false)
	{
		expected = m_lockFlag.load() & eLockFlag::ON_READ;
		desired = expected + 1;
	}
}

void RWLock::ReadUnlock()
{
	LLockOrderChecker->Pop(this);
	m_lockFlag.fetch_sub(1);
}

RWLock::RWLock(uint32_t order)
	:m_lockFlag(0), m_order(order)
{
}
