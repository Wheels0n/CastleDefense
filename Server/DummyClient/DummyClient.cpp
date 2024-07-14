#pragma once
#pragma comment(lib, "ws2_32.lib")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>

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

	
	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(777);
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);


	result = connect(hSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
	if (result == SOCKET_ERROR)
	{
		std::cout << "connect failed" << std::endl;
		return -1;
	}

	std::cout << "Connected!" << std::endl;
	
	while (true)
	{
		char sendBuf[100] = "Hello?";
		char recvBuf[100];
		result = send(hSocket, sendBuf, 100, 0);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			std::cout << "Recv ErrorCode: " << error << std::endl;
		}
		result = recv(hSocket, recvBuf, 100, 0);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			std::cout << "Recv ErrorCode: " << error << std::endl;
		}
		std::cout << "Received : " << sizeof(recvBuf) << std::endl;
	}

	result = closesocket(hSocket);
	if (result == SOCKET_ERROR)
	{
		std::cout << "closesocket() Failed" << std::endl;
	}


	WSACleanup();
	std::cout << "Exiting..." << std::endl;

}