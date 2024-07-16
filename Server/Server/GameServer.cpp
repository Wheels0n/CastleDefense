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
	char sendBuf[100];
	char recvBuf[100];
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

	std::vector<SOCKET> sockets;
	std::vector<WSAEVENT> events;

	WSAEVENT listenEvent = WSACreateEvent();
	events.push_back(listenEvent);
	sockets.push_back(listenSocket);

	result = WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT);
	if (result != 0)
	{
		std::cout << "wsaselect to accept failed()" << std::endl;
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			std::cout << "ErrorCode: " << error << std::endl;
		}
	}

	std::vector<Session> sessions;
	while (true)
	{
		
		int idx = WSAWaitForMultipleEvents(events.size(), &events[0], FALSE, INFINITE, FALSE);
		if (idx == WSA_WAIT_FAILED)
		{
			int error = WSAGetLastError();
			std::cout << "ErrorCode: " << error << std::endl;
			break;
		}
		else
		{
			idx -= WSA_WAIT_EVENT_0;
			WSANETWORKEVENTS networkEvents;
			result = WSAEnumNetworkEvents(sockets[idx], events[idx], &networkEvents);
			if (result == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				std::cout << "Recv ErrorCode: " << error << std::endl;
				break;

			}

			if (networkEvents.lNetworkEvents & FD_ACCEPT && networkEvents.iErrorCode[FD_ACCEPT_BIT] == 0)
			{
				sockaddr_in clientAddr;
				memset(&clientAddr, 0, sizeof(sockaddr_in));
				int addrLen = sizeof(sockaddr_in);
				SOCKET clientSocket;

				clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
				if (clientSocket == INVALID_SOCKET)
				{
					int error = WSAGetLastError();
					std::cout << "Accept ErrorCode: " << error << std::endl;
					break;
				}
				std::cout << "Client Connected" << std::endl;

				WSAEVENT clientEvent = WSACreateEvent();
				sockets.push_back(clientSocket);
				events.push_back(clientEvent);

				std::string idxStr = std::to_string(sessions.size());
				Session session;
				strncpy(session.sendBuf,idxStr.c_str(), idxStr.size());
				session.sendBuf[idxStr.size()] = '\0';
				sessions.push_back(session);
				result = WSAEventSelect(clientSocket, clientEvent, FD_READ| FD_WRITE);
				if (result != 0)
				{
					int error = WSAGetLastError();
					std::cout << "ErrorCode: " << error << std::endl;
					break;
				}
			}
			//클라이언트는 Send-Recv순서를 지키는데 FD_READ만 켜진경우 읽기만 하면 읽기 이후에 어떠한 플래그도 안켜져서
			//먹통이 될 수있다.
			if ((networkEvents.lNetworkEvents & (FD_READ)) && (networkEvents.iErrorCode[FD_READ_BIT] == 0)
				|| ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] == 0)))
			{
				Session& session = sessions[idx - 1];
				result = recv(sockets[idx], session.recvBuf, 100, 0);
				if (result == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					std::cout << "Recv ErrorCode: " << error << std::endl;
					break;

				}
				std::cout << session.recvBuf << std::endl;
				std::cout << "Received : " << sizeof(session.recvBuf) << std::endl;

				result = send(sockets[idx], session.sendBuf, 100, 0);
				if (result == SOCKET_ERROR)
				{
					int error = WSAGetLastError();
					std::cout << "Send ErrorCode: " << error << std::endl;
					break;
				}

				std::cout << "Send Data" << std::endl;
			}

		}
	}
	
	for (int i = 0; i < sockets.size(); ++i)
	{
		result = closesocket(sockets[i]);
		if (result == SOCKET_ERROR)
		{
			std::cout << "closesocket() Failed" << std::endl;
		}
		
		WSACloseEvent(events[i]);
	}
	
	WSACleanup();
	std::cout << "Exiting..." << std::endl;
	return 0;
}