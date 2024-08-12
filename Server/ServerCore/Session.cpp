#include "stdafx.h"
#include "CircularBuffer.h"
#include "Exception.h"
#include "Session.h"
#include "SessionManager.h"
#include "IocpManager.h"
#include "Memory.h"


extern const int SERVER_PORT;
extern const wchar_t* SERVER_ADDR;

ObjectPool<OverlappedEx>* s_pOverlappedExPool = new ObjectPool<OverlappedEx>;
void Session::AddRef()
{
	m_nRef.fetch_add(1);
}

void Session::ReleaseRef()
{
	m_nRef.fetch_sub(1);
	if (m_bConnected.load()==false&& m_nRef.load()==0)
	{
		g_pSessionManager->ReturnSession(this);
	}
}


Session::Session()
	:m_socket(INVALID_SOCKET), m_nRef(0), m_bConnected(false), m_recvBuf(nullptr), m_sendBuf(nullptr)
{
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));
	m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	//TODO : 소켓 생성 실패 시?

	m_recvBuf = MakeShared<CircularBuffer>();
	m_sendBuf = MakeShared<CircularBuffer>();
}

Session::~Session()
{
}

void Session::ResetSession()
{
	m_bConnected.store(false);
	m_nRef.store(0);
	memset(&m_sockaddr, 0, sizeof(SOCKADDR_IN));

	//바로 사용할 수 있게 TIME_WAIT를 없애 버림
	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	if (setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)) == SOCKET_ERROR)
	{
		PrintError("setsockopt()");
	}

}

bool Session::RequestAccept()
{
	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::ACCEPT;
	pOverlappedEx->m_owningSession = this;

	AddRef();

	if (IocpManager::AcceptEx(*g_pIocpManager->GetListenSocket(), m_socket, m_recvBuf->GetBuf(), 0,
		sizeof(sockaddr_in)+16, sizeof(sockaddr_in)+16,
		&bytes, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx)) == FALSE)
	{
		int errorno = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PrintError("AcceptEx()");
			return false;
		}
	}

	return true;
}

void Session::ProcessAccept()
{
	if (m_bConnected.exchange(true))
	{
		//TODO :  크래시. 연결에 연결
	}
	
	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<char*>(g_pIocpManager->GetListenSocket()), sizeof(SOCKET))
		== SOCKET_ERROR)
	{
		PrintError("setsockopt()");
	}


	int addrLen = sizeof(sockaddr_in);
	
	if(getpeername(m_socket, OUT reinterpret_cast<sockaddr*>(&m_sockaddr), &addrLen)== SOCKET_ERROR)
	{
		PrintError("getpeername()");
	}

	g_pIocpManager->RegisterSocket(m_socket);

	char addrArr[100] = { 0, };
	InetNtopA(AF_INET, &m_sockaddr.sin_addr, OUT addrArr, sizeof(addrArr));

	cout << "Client Connected: IP = " << addrArr << ", PORT = "
		<< ntohs(m_sockaddr.sin_port) << endl;

	RequestRecv();
}

bool Session::RequestDisconnect()
{
	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::DISCONNECT;
	pOverlappedEx->m_owningSession = this;

	AddRef();

	if (IocpManager::DisconnectEx(m_socket, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx), TF_REUSE_SOCKET, 0) == FALSE)
	{
		int errorno = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PrintError("DisconnectEx()");
			return false;
		}
	}

	return false;
}

void Session::ProcessDisconnect()
{
	if (m_bConnected.exchange(false)==false)
	{
		//TODO :  크래시. 해제에 해제
	}

	char addrArr[100] = { 0, };
	InetNtopA(AF_INET, &m_sockaddr.sin_addr, OUT addrArr, sizeof(addrArr));

	cout << "Client Disconnected: IP = " << addrArr << ", PORT = "
		<< ntohs(m_sockaddr.sin_port) << endl;

	ResetSession();
}

bool Session::RequestRecv()
{	
	AddRef();

	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::RECV;
	pOverlappedEx->m_owningSession = this;

	WSABUF wsabuf;
	wsabuf.buf = m_recvBuf->GetBuf();
	wsabuf.len = m_recvBuf->CalFreeSpace();

	if (WSARecv(m_socket, &wsabuf, 1, &bytes, &flags, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx), NULL) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PrintError("WSARecv()");
			return false;
		}
	}

	return true;
}

void Session::ProcessRecv(int recvlen)
{
	m_recvBuf->MoveWritePos(recvlen);
	cout << m_recvBuf->GetBuf() << endl;
	

	memcpy(m_sendBuf->GetBuf(), m_recvBuf->GetBuf(), recvlen * sizeof(char));
	m_recvBuf->MoveReadPos(recvlen);
	m_sendBuf->MoveWritePos(recvlen);

	RequestSend(recvlen);
	RequestRecv();
}

bool Session::RequestSend(int recvlen)
{
	AddRef();
	
	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::SEND;
	pOverlappedEx->m_owningSession = this;

	WSABUF wsabuf;
	wsabuf.buf = m_sendBuf->GetBuf();
	wsabuf.len = recvlen;

	if (WSASend(m_socket, &wsabuf, 1, &bytes, flags, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx), NULL) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			PrintError("WSASend()");
			return false;
		}
	}


	return false;
}

void Session::ProcessSend(int sendLen)
{
	m_sendBuf->MoveReadPos(sendLen);
}
