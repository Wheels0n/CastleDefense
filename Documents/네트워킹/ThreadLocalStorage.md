# ThreadLocalStorage

스택/힙 말고 또 다른 스레드만의 저장공간을 알아보자.

## TLS

프로세스의 모든 스레드는 해당 가상 주소 공간을 공유한다. 함수의 지역 변수는 스택에 저장되어  
각 스레드에 고유하다. 그러나 정적 변수와 전역 변수는 프로세스 내 모든 스레드에서 공유된다.  
TLS라는 공간이 따로 있다. 이름 그대로 한 스레드에 한정된 전역적 메모리 공간이다. 이를 이용하면  
힙에서 공유할 데이터를 한번에 많이 가져와서 경합을 줄일 수 있다.

### c++11

c++11에서도 표준에 도입되었다. thread_local 자료형 으로 선언한다.

```c++


// C++ Program to implement
// thread_local Storage
#include <iostream>
#include <thread>
using namespace std;

thread_local int counter = 0;

void increment_counter()
{
    counter++;
    cout << "Thread " << this_thread::get_id()
         << " counter = " << counter << endl;
}

int main()
{
    // Create first thread
    thread t1(increment_counter);
    // Create second thread
    thread t2(increment_counter);
    // Wait for the first thread to finish
    t1.join();
    // Wait for the second thread to finish
    t2.join();
    return 0;
}
```

#### 참조

- [위키피디아:Thread-local storage](https://en.wikipedia.org/wiki/Thread-local_storage)
- [MSDN: 스레드 로컬 스토리지](https://learn.microsoft.com/ko-kr/windows/win32/procthread/thread-local-storage)
- [GG: tls c++11](https://www.geeksforgeeks.org/thread_local-storage-in-cpp-11/)
