#include "stdafx.h"
#include "SessionManager.h"
#include "Session.h"

SessionManager* g_pSessionManager = nullptr;

void SessionManager::PrepareSessions()
{
	for (int i = 0; i<MAX_CONNECTION; ++i)
	{
		Session* pSession = xnew<Session>();
		m_sessions.push_back(pSession);
	}
}

void SessionManager::AcceptSessions()
{	
	WriteLockGuard wLockGuard(m_rwLock);

	while (m_nCurSessions.load() < MAX_CONNECTION)
	{
		Session* pSession = m_sessions.back();
		m_sessions.pop_back();
		m_nCurSessions.fetch_add(1);
		
		if (pSession->RequestAccept()==false)
		{
			break;
		}
	}

	return;
}

void SessionManager::ReturnSession(Session* pSession)
{

	WriteLockGuard wLockGuard(m_rwLock);
	pSession->ResetSession();
	m_sessions.push_back(pSession);
	m_nCurSessions.fetch_sub(1);
	
}

SessionManager::SessionManager()
	:m_rwLock(0)
{
	
}

SessionManager::~SessionManager()
{
}
