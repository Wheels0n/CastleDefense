#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Debug\\ServerCore.lib")
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>

#include "Memory.h"
#include "Allocator.h"
#include "ThreadPool.h"
struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuf[100] = {};
};

enum eIO_TYPE
{
	NONE,
	READ,
	WRITE,
	ACCEPT,
	CONNECT
};

struct OverlappedEx
{
	WSAOVERLAPPED overlapped = {};
	eIO_TYPE ioType = NONE;
};

void WorkerThreadMain(HANDLE hIocp)
{
	while (true)
	{
		DWORD transferredBytes=0;
		Session* pSession = nullptr;
		OverlappedEx* pOverlappedEx = nullptr;

		BOOL ret = ::GetQueuedCompletionStatus(hIocp, &transferredBytes,
			(ULONG_PTR*)&pSession, (LPOVERLAPPED*)&pOverlappedEx, INFINITE);

		if (ret == FALSE || transferredBytes ==0)
		{
			continue;
		}

		std::cout << transferredBytes << std::endl;

		WSABUF wsaBuf;
		wsaBuf.buf = pSession->recvBuf;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;

		WSARecv(pSession->socket, &wsaBuf, 1, &recvLen, &flags, &pOverlappedEx->overlapped, NULL);
	}
}



int main()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		return -1;
	}

	SOCKET  listenSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		std::cout << "socket() Failed" << std::endl;
		return -1;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(777);
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

	result = ::bind(listenSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
	if (result == SOCKET_ERROR)
	{
		std::cout << "bind() Failed" << std::endl;
		return -1;
	}

	result = listen(listenSocket, 5);
	if (result == SOCKET_ERROR)
	{
		std::cout << "listen() Failed" << std::endl;
		return -1;
	}


	HANDLE hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	ThreadPool* pThreadPool = new ThreadPool();
	for (int i = 0; i < 5; ++i)
	{
		pThreadPool->EnqueueTask([=]() { WorkerThreadMain(hIocp); });
	}

	g_pMemoryPoolManager = new MemoryPoolManager();

	std::vector<Session*> sessionManager;

	while (true)
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = INVALID_SOCKET;

		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			return -1;
		}

		Session* pSession = xnew<Session>();
		pSession->socket = clientSocket;
		sessionManager.push_back(pSession);

		std::cout << "Client Connected!" << std::endl;
		CreateIoCompletionPort((HANDLE)clientSocket, hIocp, (ULONG_PTR)pSession, 0);


		WSABUF wsaBuf;
		wsaBuf.buf = pSession->recvBuf;
		wsaBuf.len = 100;

		OverlappedEx* pOverlappedEx = xnew<OverlappedEx>();
		pOverlappedEx->ioType = READ;

		DWORD recvLen = 0;
		DWORD flags = 0;
		
		WSARecv(pSession->socket, &wsaBuf, 1, &recvLen, &flags, &pOverlappedEx->overlapped, NULL);
		
	}

	closesocket(listenSocket);
	WSACleanup();
	std::cout << "Exiting..." << std::endl;
	return 0;
}