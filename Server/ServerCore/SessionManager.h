#pragma once
#include "Allocator.h"
#include "Lock.h"
#include "stdafx.h"
const int MAX_CONNECTION = 1000;

class C_Packet;
class SendBuffer;
class Session;
class SessionManager
{
public:
	void PrepareSessions();
	
	void AcceptSessions();
	Session* ConnectSession();
	void DisconnectSession();
	void ReturnSession(Session*);
	void Brodcast(shared_ptr<SendBuffer>);
	SessionManager();
	~SessionManager();
private:
	//Ǯ���� �ϸ� ���� ref�� �ǵ�����ؼ� ������ ���������. ���� ���� ī������ ������ �ִ�.  
	//shared_ptr�� ���� �ٷ� ������ ������, ��� ����,����,������ �ؾ��Ѵ�.  
	xlist<Session*> m_sessionPool;
	xvector<Session*>m_sessionVec;
	RWLock m_rwLock;
	atomic<int> m_nCurSessions;
};

extern SessionManager* g_pSessionManager;
