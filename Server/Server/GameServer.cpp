#pragma once
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Debug\\ServerCore.lib")
#define _CRT_SECURE_NO_WARNINGS

#include "Memory.h"
#include "Session.h"
#include "SessionManager.h"
#include "IocpManager.h"

int main()
{
	//TODO : 技记 概聪历尔 IOCP概聪历甫 积己
	g_pMemoryPoolManager = new MemoryPoolManager();
	g_pSessionManager = new SessionManager();
	g_pIocpManager = new IocpManager();

	if (!g_pIocpManager->Init())
	{
		return -1;
	}
	
	
	g_pIocpManager->RunIOThreads();
	g_pIocpManager->StartAccept();

	delete g_pIocpManager;
	delete g_pSessionManager;
	delete g_pMemoryPoolManager;

	return 0;
}