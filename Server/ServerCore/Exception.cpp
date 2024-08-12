#include "stdafx.h"
#include "Exception.h"

void PrintError(const char* pFunctionName)
{
	int error = 0;
	error = WSAGetLastError();
	std::cout << pFunctionName << "Failed :" << error << std::endl;
}