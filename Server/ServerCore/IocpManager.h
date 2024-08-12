#pragma once
#include <WinSock2.h>
#include <MSWSock.h>

class IocpManager
{
public:
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_ACCEPTEX AcceptEx;
public:
	bool Init();
	void Destroy();

	void StartAccept();
	void RunIOThreads();
	void IOThreadMain(HANDLE hIocp);
	void RegisterSocket(SOCKET hSock);

	SOCKET* GetListenSocket() { return &m_hListenSock; };

	IocpManager();
	~IocpManager();
private:
	bool GetWindowFunction(GUID guid, LPVOID* pFn);

private:
	HANDLE m_hIocp;
	SOCKET m_hListenSock;

};

extern IocpManager* g_pIocpManager;

