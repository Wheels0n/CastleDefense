#pragma once
#include <WinSock2.h>
#include <MSWSock.h>

class ThreadPool;
class IocpManager
{
public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;
public:
	bool Init();
	void Destroy();

	bool StartListen();
	void StartAccept();
	void StartConnect();
	void StartDisconnect();
	void RunIOThreads();
	void StopIOThreads();
	void IOThreadMain(HANDLE hIocp);
	void RegisterSocket(SOCKET hSock);

	SOCKET* GetListenSocket() { return &m_hListenSock; };
	sockaddr_in* GetServerSockaddr() { return &m_sockaddr; };

	IocpManager();
	~IocpManager();
private:
	bool GetWindowFunction(SOCKET dummySock, GUID guid, LPVOID* pFn);

private:
	ThreadPool* m_pThreadPool;

	HANDLE m_hIocp;
	SOCKET m_hListenSock;
	sockaddr_in m_sockaddr;
};

extern IocpManager* g_pIocpManager;

