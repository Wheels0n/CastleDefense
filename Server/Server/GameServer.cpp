#pragma once
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <vector>

struct Session
{
	SOCKET socket = INVALID_SOCKET;
	char recvBuf[100] = {};
	WSAOVERLAPPED overlapped = {};
};

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

	u_long mode = 1;
	result = ioctlsocket(listenSocket, FIONBIO, &mode);
	if (result == SOCKET_ERROR)
	{
		std::cout << "ioctlsocket() Failed" << std::endl;
		return -1;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(777);
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

	result = bind(listenSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
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


	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(clientAddr);
	SOCKET clientSocket = INVALID_SOCKET;

	while (true)
	{
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket != INVALID_SOCKET)
		{
			break;
		}

	}
	
	Session session = Session{ clientSocket };
	WSAEVENT wsaEvent = WSACreateEvent();
	session.overlapped.hEvent = wsaEvent;

	while (true)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = session.recvBuf;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;
		
		if (WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, nullptr) == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSA_IO_PENDING)
			{
				WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(session.socket, &session.overlapped, &recvLen, FALSE, &flags);
				std::cout << recvLen << std::endl;
			}
		}
		else
		{
			std::cout << recvLen << std::endl;
		}
	}

	closesocket(session.socket);
	closesocket(listenSocket);
	WSACloseEvent(wsaEvent);
	WSACleanup();
	std::cout << "Exiting..." << std::endl;
	return 0;
}