#pragma once

class LockOrderChecker;
extern thread_local int LThreadId;
extern thread_local LockOrderChecker* LLockOrderChecker;