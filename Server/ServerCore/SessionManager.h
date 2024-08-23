#pragma once
#include "Allocator.h"
#include "Lock.h"
#include "stdafx.h"
const int MAX_CONNECTION = 1000;

class Session;
class SessionManager
{
public:
	void PrepareSessions();
	
	void AcceptSessions();
	void ConnectSession();
	void DisconnectSession();
	void ReturnSession(Session*);

	SessionManager();
	~SessionManager();
private:
	//풀링을 하면 직접 ref를 건드려야해서 관리가 힘들어진다. 또한 이중 카운팅의 문제도 있다.  
	//shared_ptr로 쓰고 바로 버리면 쉽지만, 계속 생성,삽입,삭제를 해야한다.  
	xlist<Session*> m_sessions;
	RWLock m_rwLock;
	atomic<int> m_nCurSessions;
};

extern SessionManager* g_pSessionManager;
