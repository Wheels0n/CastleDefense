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
		SessionManager::GetInstance().ReturnSession(shared_from_this());
	}
}

void Session::ResetSession()
{
	m_bConnected.store(false);
	m_nRef.store(0);
	memset(&m_sockaddr, 0, sizeof(SOCKADDR_IN));

	//바로 사용할 수 있게 TIME_WAIT를 없애 버림
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
	pOverlappedEx->m_owningSession = shared_from_this();

	AddRef();

	if (IocpManager::AcceptEx(*(IocpManager::GetInstance().GetListenSocket()), m_socket, m_recvBuf->GetBufEnd(), 0,
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
		//TODO :  크래시. 연결에 연결
	}
	SessionManager::GetInstance().IncreaseConnectionCount();
	
	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<char*>(IocpManager::GetInstance().GetListenSocket()), sizeof(SOCKET))
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

	IocpManager::GetInstance().RegisterSocket(m_socket);

	char addrArr[100] = { 0, };
	InetNtopA(AF_INET, &m_sockaddr.sin_addr, OUT addrArr, sizeof(addrArr));

	cout << "Client Connected: IP = " << addrArr << ", PORT = "
		<< ntohs(m_sockaddr.sin_port) << "\n";

	RequestRecv();
}

bool Session::PrepareConnect()
{
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = 0;
	InetPtonA(AF_INET, SERVER_ADDR, OUT & m_sockaddr.sin_addr);

	if (::bind(m_socket, reinterpret_cast<sockaddr*>(& m_sockaddr), sizeof(sockaddr_in))==SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("bind()", errorno);
		return false;
	}

	IocpManager::GetInstance().RegisterSocket(m_socket);

	return true;
}
bool Session::RequestConnect()
{
	// TODO: 실패시 오버랲드EX 누수와,  소켓 재등록 문제를 해결 할 것
	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::CONNECT;
	pOverlappedEx->m_owningSession = shared_from_this();

	AddRef();
	//TODO : 매개변수 에러
	if (IocpManager::ConnectEx(m_socket, reinterpret_cast<sockaddr*>(IocpManager::GetInstance().GetServerSockaddr()),
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
		//TODO :  크래시. 해제에 해제
	}

	if (setsockopt(m_socket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,NULL, 0)== SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("setsockopt()", errorno);
	}

	RequestRecv();
}

bool Session::RequestDisconnect()
{
	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::DISCONNECT;
	pOverlappedEx->m_owningSession = shared_from_this();

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
		//TODO :  크래시. 해제에 해제
	}

	char addrArr[100] = { 0, };
	InetNtopA(AF_INET, &m_sockaddr.sin_addr, OUT addrArr, sizeof(addrArr));

	cout << "Client Disconnected: IP = " << addrArr << ", PORT = "
		<< ntohs(m_sockaddr.sin_port) << "\n";

	ResetSession();
}

bool Session::RequestRecv()
{	
	AddRef();

	OverlappedEx* pOverlappedEx = s_pOverlappedExPool->Pop();

	DWORD bytes = 0;
	DWORD flags = 0;
	pOverlappedEx->m_ioType = eIO_TYPE::RECV;
	pOverlappedEx->m_owningSession =  shared_from_this();

	//끊겨서 온게 있다면 앞으로 당김
	if (m_recvBuf->CalSize())
	{
		m_recvBuf->ShiftBufferForward();
	}
	m_recvBuf->Reset();
	WSABUF wsabuf;
	//쓰는 위치로
	wsabuf.buf = m_recvBuf->GetBufEnd();
	wsabuf.len = m_recvBuf->CalFreeSpace();
	
	assert(wsabuf.len !=0);
	if (WSARecv(m_socket, &wsabuf, 1, &bytes, &flags, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx), NULL) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		if (errorno != WSA_IO_PENDING)
		{
			ReleaseRef();
			s_pOverlappedExPool->Push(pOverlappedEx);
			int errorno = WSAGetLastError();
			PrintError("WSARecv()", errorno);
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
	//방금 수신한 바이트 크기만큼 write 
	m_recvBuf->MoveWritePos(recvlen);
	if (m_recvBuf->CalSize() >= (sizeof(PacketHeader)))
	{
		{
			//recv는 최대한 많이 받아서 딱딱 안받아짐에 유의

			PacketHeader* ptr = reinterpret_cast<PacketHeader*>(m_recvBuf->GetBufBegin());
			int payloadSize = ptr->size;
			assert(payloadSize <= BUF_SIZE);
				
			//페이로드가 완성되었으면 처리.
			if (m_recvBuf->CalSize() >= payloadSize)
			{
				PacketHandler::ProcessPacket(ptr, shared_from_this());
				//완전한 패킷을 읽음
				m_recvBuf->MoveReadPos(payloadSize);
			}
		}

	}
	
	RequestRecv();
}

bool Session::RequestSend(shared_ptr<SendBuffer> pSbuffer)
{
	xvector<WSABUF> wsabufs;
	OverlappedEx* pOverlappedEx;
	{
		WriteLockGuard writelockGuard(m_sendLock);
		m_sendQueue.push(pSbuffer);
		if (m_bSending.exchange(true) == true)
		{
			return true;
		}

		pOverlappedEx = s_pOverlappedExPool->Pop();
		while (!m_sendQueue.empty())
		{
			shared_ptr<SendBuffer> cur = m_sendQueue.front();
			m_sendQueue.pop();
			pOverlappedEx->m_sendBuffers.push_back(cur);

			WSABUF wsabuf;
			wsabuf.buf = cur->GetBuffer();
			wsabuf.len = cur->GetSize();

			wsabufs.push_back(wsabuf);
		}
	}
		//몰아서 처리하니까 한번만 Ref하도록 함
		AddRef();
		
		DWORD bytes = 0;
		DWORD flags = 0;
		pOverlappedEx->m_ioType = eIO_TYPE::SEND;
		pOverlappedEx->m_owningSession = shared_from_this();
	

		if (WSASend(m_socket, &wsabufs[0], wsabufs.size(), &bytes, flags, reinterpret_cast<LPOVERLAPPED>(pOverlappedEx), NULL) == SOCKET_ERROR)
		{
			int errorno = WSAGetLastError();
			if (errorno != WSA_IO_PENDING)
			{
				ReleaseRef();
				s_pOverlappedExPool->Push(pOverlappedEx);
				m_bSending.store(false);
				int errorno = WSAGetLastError();
				PrintError("WSASend()", errorno);
				return false;
			}
		}
		m_bSending.store(false);

	return true;
}
void Session::ProcessSend(int sendLen)
{
	if (m_bConnected.load() == false)
	{
		return;
	}

}

Session::Session()
	:m_socket(INVALID_SOCKET), m_nRef(0), m_bConnected(false), m_recvBuf(nullptr),
	m_recvLock(1), m_sendLock(1)
{
	memset(&m_sockaddr, 0, sizeof(sockaddr_in));
	m_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	//TODO : 소켓 생성 실패 시?

	m_recvBuf = MakeShared<Buffer>();
}
Session::~Session()
{

}
