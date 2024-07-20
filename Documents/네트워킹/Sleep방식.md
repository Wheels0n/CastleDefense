# Sleep방식

스핀락에 이은 슬립방식을 적용해서 락을 만들어 보자.

## Sleep

이번에는 스핀락처럼 무한 루프 대기가 아닌 일정 시간 대기후 다시 복귀해서 확인하는  
방식을 취해본다. 만약 그래도 누가 락을 소유한다면 얄짤없다.

스케줄러는 한 스레드에게 일정 시간, 타임 슬라이스,만큼의 실행권을 부여한다. 꼭 이 시간을 다 써야  
실행권이 다른 스레드에게 넘어가는 건 아니다. I/O작업이나 동기화가 완료되기 전까지 프로세스가 대기  
해야하거나, 시스템콜을 호출하거나, Sleep을 명시적으로 호출 하는 경우다.

### 유저모드/커널모드

현대의 OS는 시스템 보호를 위해 프로세스를 유저모드와 커널모드로 나눈다. 유저모드는 시스템 접근에 제약이
있는 반면 커널은 그렇지 않다. 커널모드는 오로지 커널만 가능하다. 그래서 유저모드에 있는 프로그램이 시스템  
자원에 접근이 필요하면 시스템 콜을 한다.
![시스템 콜 흐름](https://www.cs.uic.edu/~jbell/CourseNotes/OperatingSystems/images/Chapter1/1_10_UserToKernelMode.jpg)

윈도우 커널은 인터럽트 핸들러를 준비해둔다. 그래서 인터럽트가 발생하면 CPU는 실행중인 프로그램을 멈추고,  
커널모드로 전환해서 핸들러를 실행한다. 작업이 끝나면 CPU는 기존의 프로그램을 재개한다.

## 코드

코드는 반복문안에 sleep_for를 넣어 준게 다다. 시간이 0이면 yield()란 함수와 효과가 같아진다.

```c++
#include <chrono>
class SpinLock
{
public:
	void lock()
	{
		bool expected = false;
		bool desired = true;
		while (m_bLocked.compare_exchange_strong(expected, desired)==false)
		{
			std::this_thread::sleep_for(std::chrono::seconds(0));
			expected = false;
		}
	}
    //...
```

#### 참조

- [위키피디아 : 컨텍스트 스위칭](https://en.wikipedia.org/wiki/Context_switch)
- [OSDevOrg : 컨텍스트 스위칭](https://wiki.osdev.org/Context_Switching)
- [MSDN : 컨테스트 스위칭](https://learn.microsoft.com/en-us/windows/win32/procthread/context-switches)
- [MSDN : 스케쥴링](https://learn.microsoft.com/en-us/windows/win32/procthread/scheduling)
- [위키피디아 : 시스템 콜](https://en.wikipedia.org/wiki/System_call)
- [GG: 시스템 콜](https://www.geeksforgeeks.org/introduction-of-system-call/)
- [윈도우 유저/커널 모드](https://en.wikibooks.org/wiki/Windows_Programming/User_Mode_vs_Kernel_Mode#Kernel_Mode,_Interrupts,_and_System_Calls)
- [양희재 교수 : 이중모드](https://velog.io/@codemcd/%EC%9A%B4%EC%98%81%EC%B2%B4%EC%A0%9COS-3.-%EC%9D%B4%EC%A4%91%EB%AA%A8%EB%93%9C%EC%99%80-%EB%B3%B4%ED%98%B8)
