#include "stdafx.h"
#include "Exception.h"
#include "IocpManager.h"
#include "Session.h"
#include "SessionManager.h"
#include "PacketHandler.h"
#include "ThreadPool.h"
#include "ThreadLocal.h"
#include "LockOrderChecker.h"

const int SERVER_PORT = 777;
const char* SERVER_ADDR = "127.0.0.1";

IocpManager* g_pIocpManager = nullptr;

LPFN_CONNECTEX IocpManager::ConnectEx = nullptr;
LPFN_DISCONNECTEX IocpManager::DisconnectEx = nullptr;
LPFN_ACCEPTEX IocpManager::AcceptEx = nullptr;

bool IocpManager::Init()
{	
	using namespace std;
	WSADATA wsaData;

	int result = WSAStartup(MAKEWORD(2, 2), OUT &wsaData);
	if (result != 0)
	{	
		int errorno = WSAGetLastError();
		PrintError("WSAStartUp()", errorno);
		return false;
	}

	memset(&m_sockaddr, 0, sizeof(sockaddr_in));
	m_sockaddr.sin_family = AF_INET;
	m_sockaddr.sin_port = htons(SERVER_PORT);
	InetPtonA(AF_INET, SERVER_ADDR, OUT & m_sockaddr.sin_addr);


	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIocp == INVALID_HANDLE_VALUE)
	{  
		int errorno = WSAGetLastError();
		PrintError("CreateIoCompletionPort()", errorno);
		return false;  
	}


	SOCKET dummySock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (dummySock == INVALID_SOCKET)
	{
		int errorno = WSAGetLastError();
		PrintError("socket()", errorno);
		return false;
	}

	GUID guids[] = {
		WSAID_CONNECTEX,
		WSAID_DISCONNECTEX,
		WSAID_ACCEPTEX
	};

	LPVOID* pFunctions[] = { 
		reinterpret_cast<LPVOID*>(&ConnectEx),
		reinterpret_cast<LPVOID*>(&DisconnectEx),
		reinterpret_cast<LPVOID*>(&AcceptEx)
	};
	for (int i = 0; i < sizeof(guids)/sizeof(GUID); ++i)
	{
		if (GetWindowFunction(dummySock, guids[i], pFunctions[i]) == false)
		{
			int errorno = WSAGetLastError();
			PrintError("GetWindowFunction()", errorno);
			return false;
		}
	}

	closesocket(dummySock);

	return true;
}

void IocpManager::Destroy()
{
	if (m_hListenSock != INVALID_SOCKET)
	{
		closesocket(m_hListenSock);
	}

	//WSACleanup();
	std::cout << "Closing..." << "\n";

}


bool IocpManager::StartListen()
{
	m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hListenSock == INVALID_SOCKET)
	{
		int errorno = WSAGetLastError();
		PrintError("socket()", errorno);
		return false;
	}
	
	if (::bind(m_hListenSock, (sockaddr*)&m_sockaddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("bind()", errorno);
		return false;
	}

	if (listen(m_hListenSock, MAX_CONNECTION) == SOCKET_ERROR)
	{
		int errorno = WSAGetLastError();
		PrintError("listen()", errorno);
		return false;
	}


	RegisterSocket(m_hListenSock);

	return true;
}

void IocpManager::StartAccept()
{
	LLockOrderChecker = new LockOrderChecker();
	g_pSessionManager->PrepareSessions();

	while (1)
	{
		g_pSessionManager->AcceptSessions();
		Sleep(100);
	}
	
}

void IocpManager::StartConnect()
{
	LLockOrderChecker = new LockOrderChecker();

	g_pSessionManager->ConnectSession();
}

void IocpManager::StartDisconnect()
{
	g_pSessionManager->DisconnectSession();
}

void IocpManager::RunIOThreads()
{
	m_pThreadPool = new ThreadPool();
	int nThreads = m_pThreadPool->GetNumOfThreads();
	for (int i = 1; i < nThreads; ++i)
	{
		m_pThreadPool->EnqueueTask([=]() { IOThreadMain(m_hIocp); });
	}

	m_pThreadPool->EnqueueTask([=]() {
			while (1)
			{
				std::this_thread::sleep_for(std::chrono::microseconds(200));
				if (g_pSessionManager->GetNumConnection())
				{
					PacketHandler::BrodcastS_EnemyMove();
				}
			}
		}
	);
}

void IocpManager::StopIOThreads()
{
	int nThreads = m_pThreadPool->GetNumOfThreads();
	for (int i = 0; i < nThreads; ++i)
	{
		PostQueuedCompletionStatus(m_hIocp, 0, NULL, NULL);
	}

	m_pThreadPool->Join();
}

void IocpManager::IOThreadMain(HANDLE hIocp)
{
	while (true)
	{
		DWORD transferredBytes = 0;
		PULONG_PTR key = 0;
		OverlappedEx* pOverlappedEx = nullptr;
		shared_ptr<Session> pSession = nullptr;
		BOOL ret = ::GetQueuedCompletionStatus(hIocp, &transferredBytes,
			(PULONG_PTR)&key, (LPOVERLAPPED*)&pOverlappedEx, INFINITE);
		if (key == NULL)
		{
			delete LLockOrderChecker;
			break;
		}

		pSession = pOverlappedEx->m_owningSession;

		if (ret == FALSE ||transferredBytes==0)
		{
			int errono = WSAGetLastError();		

			if (pOverlappedEx->m_ioType == eIO_TYPE::RECV|| pOverlappedEx->m_ioType == eIO_TYPE::SEND)
			{
				cout << "RequestDisconnect: " << errono << "\n";
				pSession->SetConnection(false);
				//TODO:서버에서만 호출하도록
				pSession->RequestDisconnect();
				continue;
			}
			
		}

		pSession = pOverlappedEx->m_owningSession;
		LARGE_INTEGER fq, s, e;
		double t;
		switch (pOverlappedEx->m_ioType)
		{
		case ACCEPT:
			pSession->ProcessAccept();
			break;
		case CONNECT:
			pSession->ProcessConnect();
			break;
		case RECV:
			pSession->ProcessRecv(transferredBytes);
			break;
		case SEND:
			pSession->ProcessSend(transferredBytes);
			break;
		case DISCONNECT:
			pSession->ProcessDisconnect();
			break;
		default:
			break;
		}
		
		Session::s_pOverlappedExPool->Push(pOverlappedEx);
		
		pSession->ReleaseRef();
	}
}

void IocpManager::RegisterSocket(SOCKET hSock)
{
	CreateIoCompletionPort((HANDLE)hSock, m_hIocp, (ULONG_PTR)&hSock, 0);
}

IocpManager::IocpManager()
	:m_hIocp(INVALID_HANDLE_VALUE), m_hListenSock(INVALID_SOCKET), m_pThreadPool(nullptr)
{
}

IocpManager::~IocpManager()
{
	delete LLockOrderChecker;
	Destroy();
}

bool IocpManager::GetWindowFunction(SOCKET dummySock, GUID guid, LPVOID* pFn)
{	
	DWORD bytes = 0;

	return WSAIoctl(dummySock, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid), pFn, sizeof(*pFn), OUT &bytes, NULL, NULL) != SOCKET_ERROR;
}
