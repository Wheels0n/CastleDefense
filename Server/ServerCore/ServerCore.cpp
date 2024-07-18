#include <thread>
#include <iostream>
void PrintHello()
{
	std::cout << "Hello" << std::endl;
}
int main()
{
	std::thread t(PrintHello);

	std::thread::id threadID = t.get_id();

	t.join();
}