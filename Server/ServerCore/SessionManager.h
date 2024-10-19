#pragma once
#include "Allocator.h"
#include "Lock.h"
#include "stdafx.h"
const int MAX_CONNECTION = 3;

class C_Packet;
class SendBuffer;
class Session;
class SessionManager
{
public:
	void PrepareSessions();
	
	void AcceptSessions();
	shared_ptr<Session> ConnectSession();
	void DisconnectSession();
	void IncreaseConnectionCount() { m_nCurSessions.fetch_add(1); };
	int GetNumConnection() { return m_nCurSessions.load(); };
	void ReturnSession(shared_ptr<Session>);
	void Brodcast(shared_ptr<SendBuffer>, shared_ptr<Session>);

	void MapIdToSession(int, shared_ptr<Session>);
	shared_ptr<Session> GetSessionById(int);

	SessionManager();
	~SessionManager();
private:
	//Ǯ���� �ϸ� ���� ref�� �ǵ�����ؼ� ������ ���������. ���� ���� ī������ ������ �ִ�.  
	//shared_ptr�� ���� �ٷ� ������ ������, ��� ����,����,������ �ؾ��Ѵ�.  
	xlist<shared_ptr<Session>> m_sessionPool;
	xvector<shared_ptr<Session>>m_sessionVec;
	xmap<int, shared_ptr<Session>> m_idToSession;
	RWLock m_rwLock;
	atomic<int> m_nCurSessions;
};

extern SessionManager* g_pSessionManager;
