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
	//Ǯ���� �ϸ� ���� ref�� �ǵ�����ؼ� ������ ���������. ���� ���� ī������ ������ �ִ�.  
	//shared_ptr�� ���� �ٷ� ������ ������, ��� ����,����,������ �ؾ��Ѵ�.  
	xlist<Session*> m_sessions;
	RWLock m_rwLock;
	atomic<int> m_nCurSessions;
};

extern SessionManager* g_pSessionManager;
