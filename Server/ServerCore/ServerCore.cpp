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

#include <vector>

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
	vector<A, StlAllocator<A>> vec;
	vec.emplace_back();
}