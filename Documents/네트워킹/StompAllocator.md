# StompAllocator

해제된 메모리를 가리키는 포인터 사용하거나, 캐스팅을 잘못할 경우 메모리 오염이 발생한다.  
근데 더 큰 문제는 바로 크래시가 나는 게 아니라 나중에 이상한 지점에서 문제가 발생해서 추  
적이 어렵다. 스톰프 할당자로 메모리 오염을 막아보자.

## 메모리 스톰프 현상

메모리 스톰프란 유효하지 않은 메모리에다가 각종 연산을 하는 것이다.

- 메모리 오버런 : 할당된 범위 끝을 지나서 읽기/쓰기
- 메모리 언더런 : 할당된 범위 시작 전에 읽기/쓰기 ex) Arr[-1];
- 메모리 해제 후 사용

안그래도 에러 추적이 힘든 데 대다수의 성능위주의 할당자들이 OS에 여러 페이지를 한번에 요청했다가  
필요하면 하나씩 주소를 부여한다고 한다. 그런 경우 OS에서는 실수했다고 알려 줄수가 없다.

## 스톰프 할당자

스톰프 할당자는 각기 다른 OS의 메모리 보호 기능을 사용한다. 이는 서로 다른 페이지에 다른 접근 모드를  
부여 할 수 있다. OS마다 다르지만 그래도 실행, 읽기, 쓰기, 그리고 접근 불가 모드를 제공한다.

원리는 간단하다 적어도 두 페이지를 할당해서, 하나는 실제 데이터를 그리고 다른 하나는 메모리 스톰프 현상을  
방지하기 위해 쓰는 것이다. 만약 마지막 유효 페이지를 넘어서게 되면 세그멘테이션 폴트를 발생시킨다.
인강강사는 그냥 1개를 쓰고있다.

![스톰프 할당자 페이지](https://img.blurredcode.com/img/UE%E7%9A%84Allocator%E4%BB%A5%E5%8F%8AStompAllocator%E7%9A%84%E5%AE%9E%E7%8E%B0-2024-04-13-16-12-33.png?x-oss-process=style/compress)

페이지 단위로 맞추려다 보니 잉여 공간이 생기는 문제도 있긴하다.

## 윈도우에서의 메모리 할당

강사가 바로 VirtualAlloc 사용법을 가르쳐 주긴 하는 데 HeapAlloc이란 걸 구글링하다 발견했다. 정리를  
할 필요가 생겼다.

![윈도우에서의 유저모드 메모리 요청](https://i.sstatic.net/mMHbW.png)

- malloc/new : 둘의 차이는 생성자 호출 여부이다. C/C++ 언어단에서 지원하며 VS C++에서는 내부적으로
  HeapAlloc 을 부른다.
- HeapAlloc : 힙으로 부터 메모리 블록을 할당한다. 인자로 힙 핸들을 받는 데, 이 힙을 얻는 방식이 두 가지가  
  있다.

1. GetProcessHeap() : 호출 프로세스의 기본 힙에 대한 핸들을 반환.
2. HeapAlloc() : 호출 프로세스의 가상 주소공간에 있는 페이지들로 구성 된 블록인, 프라이빗 힙을 반환한다.  
   만약 최대 크기보다 큰 힙을 요청하면 내부적으로 VirtualAlloc을 호출한다.

- VirtualAlloc() : 호출 프로세스의 가상 주소 공간 내의 페이지 주소 상태를 예약, 커밋, 변경한다. 이 함수로  
  할당 된 메모리는 자동으로 0으로 초기화 된다.

첫 번쨰 인자는 할당을 할 부분의 시작주소이다. NULL로하면 시스템에서 알아서 정해준다.  
두 번쨰 인자는 할당 크기이다. 첫 인자가 NULL이였다면 값은 다음 페이지의 경계로 반올림된다. 그게 아니면  
시작주소에서 시작주소+할당 크기 사이에 있는 페이지들중 하나 이상의 바이트라도 담고있는 것들을 포함한다.
만약 2바이트가 페이지 경계에 걸치면 두 페이지 모두 포함된다.  
세 번쨰 인자는 할당 방식이다.

- MEM_COMMIT : 지정된 예약된 페이지에 대한 메모리를 할당한다. 실제 물리 물리 페이지들은 가상 주소  
  공간이 실제로 접근 되기전까지는 할당되지 않는다.
- MEM_RESERVE : 실제 물리 메모리나 디스크 내에 페이징 파일에 할당 없이 주소 공간만 예약한다. 후에
  예약된 공간을 커밋하는 게 가능하다.
- MEM_RESET : 지정된 메모리 공간내의 데이터들이 더이상 필요없다는 표시이다. 페이징 파일로 부터 페이지를  
  읽거나 페이징 파일로 페이지를 써서는 안된다. 허나 나중에 메모리 블록이 다시 쓰일 것이며 decommited되어  
  서는 안된다.
- MEM_RESET_UNDO : MEM_RESET이 호출된 적 있던 공간에 불려진다. MEM_RESET의; 반대효과를 낸다.

네 번쨰 인자는 할당 될 페이지에 대한 보호 여부이다.

성공하면 base주소를 실패하면 NULL을 반환한다. 해제는 VirtualFree()를 통한다.

- VirtualFree() : 호출자의 가상 주소 공간 내에 페이지들을 해제, 디커밋, 또는 해제 및 디커밋 한다.

첫 번쨰 인자는 해제할 페이지 공간의 베이스 주소이다.
두 번쨰 인자는 해제할 메모리 공간의 바이트 크기이다.
세 번쨰 인자는 해제 방식이다. 방식은 다음과 같다.

- MEM_DECOMMIT : 커밋된 페이지들의 지정된 공간을 디커밋한다. 호출후에 페이지들은 물리공간을  
  해제하고 예약상태가 된다.
- MEM_RELEASE : 페이지의 지정된 부분을 해제한다. 호출후 페이지들은 free상태가 된다. 해제후에는
  차후 할당이 가능해진다.

성공시 0 이외의 값을, 실패시 0을 반환한다.

디커밋되거나 해제된 메모리는 더이상 참조가 불가능하다. 참조를 하면 메모리 접근 위반 예외가 발생한다.

## 코드

```c++
SYSTEM_INFO sysInfo;
GetSystemInfo(&sysInfo);

sysInfo.dwPageSize;
sysInfo.dwAllocationGranularity;
```

GetSystemInfo 페이지 크기와 메모리 주소 배수를 얻을 수 있다. 각각 4KB/64KB가 나왔다.

```c++
void* StompAllocator::Alloc(int size)
{
    const int nPage = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    void* pBase = VirtualAlloc(NULL, nPage * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return pBase;
}

void StompAllocator::Release(void* ptr)
{
    VirtualFree(ptr, 0, MEM_RELEASE);
}

```

코드는 별거 없고 필요한 페이지 수를 계산해서 넘기고 xnew/xdelete에 할당자를 교체한 게 다다.
해제후 접근은 잘 잡지만, 할당된 페이지 크기가 커서 메모리 오버런을 해도 바로 못잡는다.
강사는 그냥 페이지 경계에 맞도록 포인터 주소 연산을 해서 쓰라고 하는 데 일단은 지켜봐야겠다.

#### 참조

- [언리얼 엔진 할당자](https://www.blurredcode.com/2023/10/d7a3cb82/)
- [언리얼 엔진 할당자(원본)](https://web.archive.org/web/20231009164648/https://pzurita.wordpress.com/2015/06/29/memory-stomp-allocator-for-unreal-engine-4/)
- [What's the differences between VirtualAlloc and HeapAlloc?](https://stackoverflow.com/questions/872072/whats-the-differences-between-virtualalloc-and-heapalloc)
- [MSDN : HeapAlloc](https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-heapalloc)
- [MSDN : GetProcessHeap](https://learn.microsoft.com/en-us/windows/win32/api/heapapi/nf-heapapi-getprocessheap)
- [MSDN : 힙 함수들](https://learn.microsoft.com/en-us/windows/win32/Memory/heap-functions)
- [MSDN : VirtualAlloc](https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-virtualalloc)
