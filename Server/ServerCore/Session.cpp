#include "stdafx.h"
#include "CircularBuffer.h"
#include "Exception.h"
#include "Session.h"
#include "SessionManager.h"
#include "PacketHandler.h"
#include "IocpManager.h"
#include "Memory.h"


extern const int SERVER_PORT;
extern const char* SERVER_ADDR;

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
	:m_socket(INVALID_SOCKET), m_nRef(0), m_bConnected(false), m_recvBuf(nullptr), m_sendBuf(nullptr), m_writeLock(1)
{
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));
	m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	//TODO : ���� ���� ���� ��?

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

	//�ٷ� ����� �� �ְ� TIME_WAIT�� ���� ����
	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 1000;

	if (setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("setsockopt()", errorno);
	}

}

void Session::ShutdownSocket()
{
	
	if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("shutdown()", errorno);
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
		nullptr, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx)) == FALSE)
	{
		int errorno = WSAGetLastError();
		if (errorno != WSA_IO_PENDING)
		{
			ReleaseRef();
			s_pOverlappedExPool->Push(pOverlappedEx);
			int errorno = WSAGetLastError();
			PrintError("AcceptEx()", errorno);
			return false;
		}
	}

	return true;
}

void Session::ProcessAccept()
{
	if (m_bConnected.exchange(true)==true)
	{
		//TODO :  ũ����. ���ῡ ����
	}
	
	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<char*>(g_pIocpManager->GetListenSocket()), sizeof(SOCKET))
		== SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("setsockopt()", errorno);
	}


	int addrLen = sizeof(sockaddr_in);
	
	if(getpeername(m_socket, OUT reinterpret_cast<sockaddr*>(&m_sockaddr), &addrLen)== SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("getpeername()",errorno);
	}

	g_pIocpManager->RegisterSocket(m_socket);

	char addrArr[100] = { 0, };
	InetNtopA(AF_INET, &m_sockaddr.sin_addr, OUT addrArr, sizeof(addrArr));

	cout << "Client Connected: IP = " << addrArr << ", PORT = "
		<< ntohs(m_sockaddr.sin_port) << endl;

	RequestRecv();
}

bool Session::PrepareConnect()
{
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(555);
	InetPtonA(AF_INET, SERVER_ADDR, OUT & m_sockaddr.sin_addr);

	if (::bind(m_socket, reinterpret_cast<sockaddr*>(& m_sockaddr), sizeof(sockaddr_in))==SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("bind()", errorno);
		return false;
	}

	g_pIocpManager->RegisterSocket(m_socket);

	return true;
}

bool Session::RequestConnect()
{
	// TODO: ���н� �����N��EX ������,  ���� ���� ������ �ذ� �� ��
	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::CONNECT;
	pOverlappedEx->m_owningSession = this;

	AddRef();
	//TODO : �Ű����� ����
	if (IocpManager::ConnectEx(m_socket, reinterpret_cast<sockaddr*>(g_pIocpManager->GetServerSockaddr()),
		sizeof(sockaddr), nullptr, 0, &bytes, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx)) == FALSE)
	{
		int errorno = WSAGetLastError();
		if (errorno != WSA_IO_PENDING)
		{
			ReleaseRef();
			s_pOverlappedExPool->Push(pOverlappedEx);

			PrintError("ConectEx()", errorno);
			return false;
		}
	}
	return true;
}

void Session::ProcessConnect()
{	
	if (m_bConnected.exchange(true) == true)
	{
		//TODO :  ũ����. ������ ����
	}

	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,NULL, 0)== SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("setsockopt()", errorno);
	}


	Packet pkt;
	pkt.set_id(m_socket);
	pkt.set_msg("hello");
	
	int packetSize = (pkt.ByteSizeLong() + sizeof(PacketHeader));

	PacketHandler::WritePacket(pkt, m_sendBuf->GetBuf());
	m_sendBuf->MoveWritePos(packetSize);
	RequestSend(packetSize);
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
		if (errorno != WSA_IO_PENDING)
		{
			ReleaseRef();
			s_pOverlappedExPool->Push(pOverlappedEx);
			PrintError("DisconnectEx()", errorno);
			return false;
		}
	}

	return false;
}

void Session::ProcessDisconnect()
{
	if (m_bConnected.exchange(false)==false)
	{
		//TODO :  ũ����. ������ ����
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
		if (errorno != WSA_IO_PENDING)
		{
			ReleaseRef();
			s_pOverlappedExPool->Push(pOverlappedEx);
			int errorno = WSAGetLastError();
			PrintError("WSARecv()",errorno);
			return false;
		}
	}

	return true;
}

void Session::ProcessRecv(int recvlen)
{
	if (m_bConnected.load() == false)
	{
		return;
	}
	WriteLockGuard writelockGuard(m_writeLock);
	m_recvBuf->MoveWritePos(recvlen);
	
	PacketHandler::ReadPacket(m_recvBuf->GetBuf());
	void* recvBuf= m_recvBuf->GetBuf();
	PacketHeader* pHeader = reinterpret_cast<PacketHeader*>(recvBuf);

	memcpy(m_sendBuf->GetBuf(), recvBuf, pHeader->size);

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
		if (errorno != WSA_IO_PENDING)
		{
			ReleaseRef();
			s_pOverlappedExPool->Push(pOverlappedEx);
			int errorno = WSAGetLastError();
			PrintError("WSASend()", errorno);
			return false;
		}
	}


	return false;
}

void Session::ProcessSend(int sendLen)
{
	if (m_bConnected.load() == false)
	{
		return;
	}
	WriteLockGuard writelockGuard(m_writeLock);
	m_sendBuf->MoveReadPos(sendLen);
}
