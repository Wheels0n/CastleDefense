#pragma once

using namespace std;
class RWLock
{
	enum eLockFlag : uint32_t
	{
		EMPTY = 0x0000'0000,
		ON_READ  = 0x0000'FFFF,
		ON_WRITE = 0xFFFF'0000
	};

public:
	void WriteLock();
	void WriteUnlock();
	void ReadLock();
	void ReadUnlock();
	
	inline void		SetOrder(uint32_t order) { m_order = order; };
	inline uint32_t GetOrder() { return m_order; };

	RWLock(uint32_t order);
	RWLock() :m_lockFlag(0) {};
	~RWLock()=default;

private:
	atomic<uint32_t> m_lockFlag;
	uint32_t m_order;
};

class ReadLockGuard
{
public:
	ReadLockGuard(RWLock& rlock)
		:m_rlock(rlock)
	{
		m_rlock.ReadLock();
	}
	~ReadLockGuard()
	{
		m_rlock.ReadUnlock();
	}

	RWLock& m_rlock;
};

class WriteLockGuard
{
public:
	WriteLockGuard(RWLock& wlock)
		:m_wlock(wlock)
	{
		m_wlock.WriteLock();
	}
	~WriteLockGuard()
	{
		m_wlock.WriteUnlock();
	}

	RWLock& m_wlock;
};