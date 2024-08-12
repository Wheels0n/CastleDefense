#include "stdafx.h"
#include "Exception.h"
#include "IocpManager.h"
#include "Session.h"
#include "SessionManager.h"
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
	int error = 0;
	int result = WSAStartup(MAKEWORD(2, 2), OUT &wsaData);
	if (result != 0)
	{	
		PrintError("WSAStartUp()");
		return false;
	}

	m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hListenSock == INVALID_SOCKET)
	{
		PrintError("socket()");
		return false;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	InetPtonA(AF_INET, SERVER_ADDR, OUT &serverAddr.sin_addr);
	result = ::bind(m_hListenSock, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
	if (result == SOCKET_ERROR)
	{
		PrintError("bind()");
		return false;
	}

	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIocp == INVALID_HANDLE_VALUE)
	{  
		PrintError("CreateIoCompletionPort()");
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
		if (GetWindowFunction(guids[i], pFunctions[i]) == false)
		{
			PrintError("GetWindowFunction()");
			return false;
		}
	}

	g_pSessionManager->PrepareSessions();

	return true;
}

void IocpManager::Destroy()
{

	closesocket(m_hListenSock);
	WSACleanup();
	std::cout << "Closing..." << std::endl;

}


void IocpManager::StartAccept()
{
	 LLockOrderChecker = new LockOrderChecker();

	if (listen(m_hListenSock, MAX_CONNECTION) == SOCKET_ERROR)
	{
		PrintError("listen()");
		return;
	}

	RegisterSocket(m_hListenSock);

	while (1)
	{
		g_pSessionManager->AcceptSessions();
		Sleep(100);
	}
	delete LLockOrderChecker;
}

void IocpManager::RunIOThreads()
{
	ThreadPool* pThreadPool = new ThreadPool();
	int nThreads = pThreadPool->GetNumOfThreads();
	for (int i = 0; i < nThreads; ++i)
	{
		pThreadPool->EnqueueTask([=]() { IOThreadMain(m_hIocp); });
	}
}

void IocpManager::IOThreadMain(HANDLE hIocp)
{
	while (true)
	{
		DWORD transferredBytes = 0;
		DWORD key = 0;
		OverlappedEx* pOverlappedEx = nullptr;
		Session* pSession = nullptr;
		BOOL ret = ::GetQueuedCompletionStatus(hIocp, &transferredBytes,
			(ULONG_PTR*)&key, (LPOVERLAPPED*)&pOverlappedEx, INFINITE);
		
		pSession = pOverlappedEx->m_owningSession;

		if (ret == FALSE ||transferredBytes==0)
		{
			int errono = WSAGetLastError();
			
			if (pOverlappedEx->m_ioType == eIO_TYPE::RECV|| pOverlappedEx->m_ioType == eIO_TYPE::SEND)
			{
				cout << errono << endl;
				pSession->RequestDisconnect();
				continue;
			}

		}

		pSession = pOverlappedEx->m_owningSession;

		switch (pOverlappedEx->m_ioType)
		{
		case ACCEPT:
			pSession->ProcessAccept();
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
	:m_hIocp(INVALID_HANDLE_VALUE), m_hListenSock(INVALID_SOCKET)
{
}

IocpManager::~IocpManager()
{
	Destroy();
}

bool IocpManager::GetWindowFunction(GUID guid, LPVOID* pFn)
{	
	DWORD bytes = 0;

	return WSAIoctl(m_hListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&guid, sizeof(guid), pFn, sizeof(*pFn), OUT &bytes, NULL, NULL) != SOCKET_ERROR;
}
