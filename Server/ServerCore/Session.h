#pragma once

#include "Memory.h"
#include "Lock.h"
#include "SendBuffer.h"

enum eIO_TYPE
{
	NONE,
	RECV,
	SEND,
	ACCEPT,
	CONNECT,
	DISCONNECT
};

class Session;
struct OverlappedEx
{		
	OVERLAPPED m_overlapped;
	shared_ptr<Session> m_owningSession;
	eIO_TYPE m_ioType;
	xvector<shared_ptr<SendBuffer>> m_sendBuffers;
};

class Buffer; 
class Session : public enable_shared_from_this<Session>
{
public:
	void AddRef();
	void ReleaseRef();

	void ResetSession();
	void ShutdownSocket();

	bool RequestAccept();
	void ProcessAccept();

	bool PrepareConnect();
	bool RequestConnect();
	void ProcessConnect();

	bool RequestDisconnect();
	void ProcessDisconnect();

	bool RequestRecv();
	void ProcessRecv(int);

	bool RequestSend(shared_ptr<SendBuffer>);
	void ProcessSend(int);

	void  SetConnection(bool b) { m_bConnected.store(b); };
	bool  GetConnection() { return m_bConnected.load(); };

	Session();
	~Session();

public:
	static ObjectPool<OverlappedEx>* s_pOverlappedExPool;
private:

	SOCKET m_socket;
	sockaddr_in m_sockaddr;

	atomic<int> m_nRef;
	atomic<bool>m_bConnected;
	atomic<bool>m_bSending;

	shared_ptr<Buffer> m_recvBuf;
	xqueue<shared_ptr<SendBuffer>> m_sendQueue;

	RWLock m_recvLock;
	RWLock m_sendLock;
};
