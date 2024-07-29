#include "Lock.h"

void Lock::WriteLock()
{
	// r/w 둘 다 없어야 함
	uint32_t expected = eLockFlag::EMPTY;
	uint32_t desired = eLockFlag::ON_WRITE;
	while (m_lockFlag.compare_exchange_strong(expected, desired)==false)
	{
		expected = eLockFlag::EMPTY;
	}

}

void Lock::WriteUnlock()
{
	m_lockFlag.store(eLockFlag::EMPTY);
}

void Lock::ReadLock()
{
	//겹친다 지금 r/w랑 
	uint32_t expected = m_lockFlag.load()&eLockFlag::ON_READ;
	uint32_t desired = expected+1;
	while (m_lockFlag.compare_exchange_strong(expected, desired)==false)
	{
		expected = m_lockFlag.load() & eLockFlag::ON_READ;
		desired = expected + 1;
	}
}

void Lock::ReadUnlock()
{
	m_lockFlag.fetch_sub(1);
}

Lock::Lock()
	:m_lockFlag(0)
{
}
