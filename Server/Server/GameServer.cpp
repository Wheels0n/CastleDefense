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

	SOCKET  hListendSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hListendSocket == INVALID_SOCKET)
	{
		std::cout << "socket() Failed" << std::endl;
		return -1;
	}


	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(777);
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

	result = bind(hListendSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
	if (result == SOCKET_ERROR)
	{
		std::cout << "bind() Failed" << std::endl;
		return -1;
	}
	
	result =listen(hListendSocket, 5);
	if (result == SOCKET_ERROR)
	{
		std::cout << "listen() Failed" << std::endl;
		return -1;
	}

	while (true)
	{
		sockaddr_in clientAddr;
		memset(&clientAddr, 0, sizeof(sockaddr_in));
		int addrLen = sizeof(sockaddr_in);
		SOCKET clientSocket = accept(hListendSocket, (sockaddr*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();
			std::cout << "Accept ErrorCode: " << error << std::endl;
			return -1;
		}

		std::cout << "Connected!" << std::endl;

	}

	result = closesocket(hListendSocket);
	if (result == SOCKET_ERROR)
	{
		std::cout << "closesocket() Failed" << std::endl;
	}

	WSACleanup();
	std::cout << "Exiting..." << std::endl;

}