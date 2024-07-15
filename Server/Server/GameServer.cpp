#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string.h>
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

	sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(sockaddr_in));
	int addrLen = sizeof(sockaddr_in);
	SOCKET clientSocket;


	fd_set reads,cpyReads, writes, cpyWrites;
	FD_ZERO(&reads);
	FD_ZERO(&writes);

	FD_SET(listenSocket, &reads);

	result = select(0, &reads, nullptr, nullptr, nullptr);
	if (result==0)
	{
		std::cout << "select to accept failed()" << std::endl;
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			std::cout << "ErrorCode: " << error << std::endl;
		}
		return -1;
	}

	clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		std::cout << "Accept ErrorCode: " << error << std::endl;
		return -1;
		
	}
		
	std::cout << "Client Connected" << std::endl;

	FD_SET(clientSocket, &reads);
	FD_SET(clientSocket, &writes);
	cpyReads = reads;
	cpyWrites = writes;
	while (true)
	{
		reads = cpyReads;
		writes = cpyWrites;

		char sendBuf[100] = "received";
		char recvBuf[100];

		result = select(0, &reads, nullptr, nullptr, nullptr);

		if(result)
		{
			result = recv(clientSocket, recvBuf, 100, 0);
			if (result == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				std::cout << "Recv ErrorCode: " << error << std::endl;
				return -1;
				
			}
			else
			{
				std::cout << recvBuf << std::endl;
				std::cout << "Received : " << sizeof(recvBuf) << std::endl;

				result = select(0, nullptr, &writes, nullptr, nullptr);
				if(result)
				{
					result = send(clientSocket, sendBuf, 100, 0);
					if (result == SOCKET_ERROR)
					{
						int error = WSAGetLastError();
						std::cout << "Send ErrorCode: " << error << std::endl;
						return -1;
					}
					
					std::cout << "Send Data" << std::endl;
					
				}
				else
				{
					std::cout << "select to write failed()" << std::endl;
					int error = WSAGetLastError();
					std::cout << "ErrorCode: " << error << std::endl;
					return -1;
				}
			}

		}
		else
		{
			std::cout << "select to read failed()" << std::endl;
			int error = WSAGetLastError();
			std::cout << "ErrorCode: " << error << std::endl;
			return -1;
		}
	}
	
	result = closesocket(listenSocket);
	if (result == SOCKET_ERROR)
	{
		std::cout << "closesocket() Failed" << std::endl;
	}

	WSACleanup();
	std::cout << "Exiting..." << std::endl;
	return 0;
}