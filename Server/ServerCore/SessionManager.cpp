#include "stdafx.h"
#include "SessionManager.h"
#include "Session.h"
#include "test.pb.h"
SessionManager* g_pSessionManager = nullptr;

void SessionManager::PrepareSessions()
{
	for (int i = 0; i<MAX_CONNECTION; ++i)
	{
		Session* pSession = xnew<Session>();
		m_sessionPool.push_back(pSession);
		m_sessionVec.push_back(pSession);
	}
}

void SessionManager::AcceptSessions()
{	
	WriteLockGuard wLockGuard(m_rwLock);

	while (m_nCurSessions.load() < MAX_CONNECTION)
	{
		Session* pSession = m_sessionPool.back();
		m_sessionPool.pop_back();
		m_nCurSessions.fetch_add(1);
		
		if (pSession->RequestAccept()==false)
		{
			break;
		}
		
	}

	return;
}

Session* SessionManager::ConnectSession()
{
	Session* pSession = xnew<Session>();
	m_sessionPool.push_back(pSession);
	m_nCurSessions.fetch_add(1);

	if(pSession->PrepareConnect() == false)
	{
		return nullptr;
	}
	
	if (pSession->RequestConnect() == false)
	{
		return nullptr;
	}
	return pSession;

}

void SessionManager::DisconnectSession()
{
	m_nCurSessions.fetch_sub(1);
	Session* pSession = m_sessionPool.back();
	m_sessionPool.pop_back();
	pSession->ShutdownSocket();
	while (pSession->GetConnection())
	{

	}
}

void SessionManager::ReturnSession(Session* pSession)
{

	WriteLockGuard wLockGuard(m_rwLock);
	pSession->ResetSession();
	m_sessionPool.push_back(pSession);
	m_nCurSessions.fetch_sub(1);
}

void SessionManager::Brodcast(shared_ptr<SendBuffer> pSendBuffer)
{

	for (int i = 0; i < m_sessionVec.size(); ++i)
	{
		if (m_sessionVec[i]->GetConnection())
		{
			//TODO: ¿©±â ³¢¾îµé±â ¾îÂ¼Áö?
			m_sessionVec[i]->RequestSend(pSendBuffer);
		}
	}
}

SessionManager::SessionManager()
	:m_rwLock(0)
{
	
}

SessionManager::~SessionManager()
{
}
