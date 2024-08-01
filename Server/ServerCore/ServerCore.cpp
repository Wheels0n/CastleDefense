#pragma once

#include <thread>
#include <iostream>
#include <atomic>
#include <mutex>
#include <chrono>
#include <future>
#include <Windows.h>

#include "ThreadPool.h"
#include "Lock.h"

#include "Memory.h"

class A
{
public:
	A() { cout << "A Constructor" << endl; };
	~A() { cout << "A Destructor" << endl; };

};
class B : public A
{
public:
	B() { cout << "B Constructor" << endl; };
	~B() { cout << "B Destructor" << endl; };
	int num;
};


int main()
{
	A* pA = xnew<A>();
	B* pB = static_cast<B*>(pA);


	pB->num = 0;
}