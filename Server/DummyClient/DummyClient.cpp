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
	InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

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

	while(true)
	{

		char sendBuf[100] = "Hello?";
		char recvBuf[100];

		while (true)
		{
			result = send(hSocket, sendBuf, 100, 0);
			if (result == SOCKET_ERROR)
			{
				int error = WSAGetLastError();
				if (error != WSAEWOULDBLOCK)
				{
					std::cout << "Send ErrorCode: " << error << std::endl;
					return -1;
				}
			}
			else
			{
				std::cout << "Send Data" << std::endl;
				while (true)
				{
					result = recv(hSocket, recvBuf, 100, 0);
					if (result == SOCKET_ERROR)
					{
						int error = WSAGetLastError();
						if (error != WSAEWOULDBLOCK)
						{
							std::cout << "Recv ErrorCode: " << error << std::endl;
							return -1;
						}
					}
					else
					{
						std::cout << recvBuf << std::endl;
						std::cout << "Received : " << sizeof(recvBuf) << std::endl;
						break;
					}
				}
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