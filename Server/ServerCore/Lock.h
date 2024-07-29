#pragma once

#include <atomic>

using namespace std;
class Lock
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

	Lock();
	~Lock()=default;

private:
	atomic<uint32_t> m_lockFlag;
};
