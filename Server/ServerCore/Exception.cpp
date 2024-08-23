#include "stdafx.h"
#include "Exception.h"

void PrintError(const char* pFunctionName, int errorno)
{
	std::cout << pFunctionName << "Failed :" << errorno << std::endl;
}