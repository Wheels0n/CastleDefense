#pragma once


#include "Memory.h"
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
	Session* m_owningSession;
	eIO_TYPE m_ioType;
	//WSABUF 
};

class CircularBuffer; 
class Session
{
public:
	void AddRef();
	void ReleaseRef();

	void ResetSession();
	
	bool RequestAccept();
	void ProcessAccept();

	bool RequestDisconnect();
	void ProcessDisconnect();

	bool RequestRecv();
	void ProcessRecv(int);

	bool RequestSend(int);
	void ProcessSend(int);

	Session();
	~Session();

public:
	static ObjectPool<OverlappedEx>* s_pOverlappedExPool;
private:

	SOCKET m_socket;
	sockaddr_in m_sockaddr;

	atomic<int> m_nRef;
	atomic<bool>m_bConnected;

	shared_ptr<CircularBuffer> m_recvBuf;
	shared_ptr<CircularBuffer> m_sendBuf;

};
