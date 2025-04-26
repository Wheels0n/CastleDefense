#include "stdafx.h"
#include "SessionManager.h"
#include "Session.h"
#include "test.pb.h"

void				SessionManager::PrepareSessions()
{
	for (int i = 0; i<MAX_CONNECTION; ++i)
	{
		shared_ptr<Session> pSession = MakeShared<Session>();
		m_sessionPool.push_back(pSession);
		m_sessionVec.push_back(pSession);
	}
}
void				SessionManager::AcceptSessions()
{	
	WriteLockGuard wLockGuard(m_rwLock);
	if (m_sessionPool.empty())
	{
		return;
	}
	shared_ptr<Session> pSession = m_sessionPool.back();
	m_sessionPool.pop_back();

	pSession->RequestAccept();
	return;
}
shared_ptr<Session> SessionManager::ConnectSession()
{
	shared_ptr<Session> pSession = MakeShared<Session>();
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
void				SessionManager::DisconnectSession()
{
	m_nCurSessions.fetch_sub(1);
	shared_ptr<Session> pSession = m_sessionPool.back();
	m_sessionPool.pop_back();
	pSession->ShutdownSocket();
	while (pSession->GetConnection())
	{

	}
}
void				SessionManager::ReturnSession(shared_ptr<Session> pSession)
{

	WriteLockGuard wLockGuard(m_rwLock);
	pSession->ResetSession();
	m_sessionPool.push_back(pSession);
	m_nCurSessions.fetch_sub(1);
}

void				SessionManager::MapIdToSession(int id, shared_ptr<Session> pSession)
{
	m_idToSession[id] = pSession;
}
shared_ptr<Session> SessionManager::GetSessionById(int id)
{
	return m_idToSession[id];
}

void				SessionManager::Brodcast(shared_ptr<SendBuffer> pSendBuffer, shared_ptr<Session> pSession)
{
	for (int i = 0; i < m_sessionVec.size(); ++i)
	{
		if (m_sessionVec[i] != pSession && m_sessionVec[i]->GetConnection())
		{
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
