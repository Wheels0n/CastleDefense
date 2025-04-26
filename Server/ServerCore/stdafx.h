#pragma once

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Debug\\libprotobufd.lib")

#include <WinSock2.h> 
#include <windows.h> 
#include <mswsock.h>
#include <ws2tcpip.h>


#include <atomic>
#include <future>

#include <cstdlib>
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>

#include <interlockedapi.h>
#include <memory>
#include <memoryapi.h>
#include <mutex>
#include <thread>
#include <utility>

#include "XTL.h"

#define OUT
#define _STD_TIME std::chrono::time_point<std::chrono::high_resolution_clock>