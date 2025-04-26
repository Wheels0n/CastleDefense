#pragma once
#include "Allocator.h"
#include "Lock.h"
#include "stdafx.h"
static const int MAX_CONNECTION = 3;

class C_Packet;
class SendBuffer;
class Session;
class SessionManager
{
private:

						SessionManager();
						SessionManager(const SessionManager& obj) = delete;
public:
	void				PrepareSessions();
	void				AcceptSessions();
	shared_ptr<Session> ConnectSession();
	void				DisconnectSession();
	void				ReturnSession(shared_ptr<Session>);

	void				IncreaseConnectionCount() { m_nCurSessions.fetch_add(1); };
	int					GetNumConnection() { return m_nCurSessions.load(); };
	
	void				MapIdToSession(int, shared_ptr<Session>);
	shared_ptr<Session> GetSessionById(int);

	void				Brodcast(shared_ptr<SendBuffer>, shared_ptr<Session>);
	

	static SessionManager& GetInstance()
	{
		static SessionManager instance;
		return instance;
	}
						~SessionManager();
private:
	//Ǯ���� �ϸ� ���� ref�� �ǵ�����ؼ� ������ ���������. ���� ���� ī������ ������ �ִ�.  
	//shared_ptr�� ���� �ٷ� ������ ������, ��� ����,����,������ �ؾ��Ѵ�.  
	xlist<shared_ptr<Session>>		m_sessionPool;
	xvector<shared_ptr<Session>>	m_sessionVec;
	xmap<int, shared_ptr<Session>>	m_idToSession;
	atomic<int>						m_nCurSessions;

	RWLock							m_rwLock;
};
