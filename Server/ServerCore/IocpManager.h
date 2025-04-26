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

private:
	bool			GetWindowFunction(SOCKET dummySock, GUID guid, LPVOID* pFn);
					IocpManager();
					IocpManager(const IocpManager& obj) = delete;
public:
	bool			Init();
	void			Destroy();

	bool			StartListen();
	void			StartConnect();
	void			StartDisconnect();

	void			RunIOMain();
	void			RunIOThreads();
	void			StopIOThreads();
	void			IOThreadMain(HANDLE hIocp);


	void			RegisterSocket(SOCKET hSock);
	SOCKET*			GetListenSocket()	{ return &m_hListenSock; };
	sockaddr_in*	GetServerSockaddr() { return &m_sockaddr; };

					~IocpManager();
					static IocpManager& GetInstance()
					{
						static IocpManager instance;
						return instance;
					}
private:
	HANDLE			m_hIocp;

	ThreadPool*		m_pThreadPool;

	SOCKET			m_hListenSock;
	sockaddr_in		m_sockaddr;
};

