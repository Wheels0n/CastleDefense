#include "ThreadLocal.h"

thread_local int LThreadId = 0;
thread_local LockOrderChecker* LLockOrderChecker = nullptr;