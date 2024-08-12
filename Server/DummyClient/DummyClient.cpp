#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cstdlib>

int main()
{
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		std::cout << "WSAStartup() Failed" << std::endl;
		return -1;
	}

	SOCKET  hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		std::cout << "socket() Failed" << std::endl;
		return -1;
	}

	u_long mode = 1;
	result = ioctlsocket(hSocket, FIONBIO, &mode);
	if (result == SOCKET_ERROR)
	{
		std::cout << "ioctlsocket() Failed" << std::endl;
		return -1;
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(777);
	InetPtonA(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	while (true)
	{
		result = connect(hSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEISCONN)
			{
				break;
			}
			else if (error == WSAEALREADY)
			{
				continue;
			}
			else if (error != WSAEWOULDBLOCK)
			{
				std::cout << "connect ErrorCode: " << error << std::endl;
				return -1;
			}
		}
	}
	

	std::cout << "Connected!" << std::endl;
	char sendBuf[100] = "Hello";
	char recvBuf[100] = { 0, };
	WSAOVERLAPPED overlapped = {};
	WSAEVENT wsaEvent = WSACreateEvent();
	overlapped.hEvent = wsaEvent;

	while(true)
	{

		DWORD bytes = 0;
		DWORD flags = 0;

		WSABUF sendWsaBuf;
		sendWsaBuf.buf = sendBuf;
		sendWsaBuf.len = 6;

		if (WSASend(hSocket, &sendWsaBuf, 1, &bytes, flags, &overlapped, nullptr) == SOCKET_ERROR)
		{
			int errorno = WSAGetLastError();
			if (WSAGetLastError() == WSA_IO_PENDING)
			{
				WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(hSocket, &overlapped, &bytes, FALSE, &flags);
			}
		}

		WSABUF recvWsaBuf;
		recvWsaBuf.buf = recvBuf;
		recvWsaBuf.len = bytes;
		if (WSARecv(hSocket, &recvWsaBuf, 1, &bytes, &flags, &overlapped, nullptr) == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSA_IO_PENDING)
			{
				WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
				WSAGetOverlappedResult(hSocket, &overlapped, &bytes, FALSE, &flags);
				std::cout << recvBuf << std::endl;
			}
		}
		
	}

	result = closesocket(hSocket);
	if (result == SOCKET_ERROR)
	{
		std::cout << "closesocket() Failed" << std::endl;
	}


	WSACleanup();
	std::cout << "Exiting..." << std::endl;
	
	return 0;
}