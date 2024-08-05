# Overlapped 모델

비동기-넌블락 오버랲드 모델을 도입해보자

## 동기/비동기 vs 블락/넌블락

이 둘이 같다고 착각하기 쉬운데 서로 다르다.

![동기/비동기 vs 블락/넌블락](https://images.velog.io/images/kjh3865/post/c7021130-58df-44e8-813f-466bafac56d6/2021-03-07T20_37_39.png)

- 블락/넌 블락 : 함수 호출시 대기하는 지 바로 완료인지
- 동기/비동기 : 요청한 동작이 꼭 동시에 일어나는 지 유무

지금까지는 select나 WSAEventSelect같은 함수로 조건이 충족 되었는 지 확인하고 나서 send/recv했다면  
이제는 작업을 예약을 하고 나중에 준비되면 실행 하게 할 것이다. 완료 통지 방식은 이벤트/콜백이 있다.

## 오버랲드 함수

오버랲드 함수들은 다음과 같다.

- WSASend
- WSARecv
- AcceptEx
- ConnectEx

다만 아래 2개는 준비 작업이 필요해서 나중에 다루겠다.

### WSASend

첫 번쨰 인자는 소켓 핸들이다.  
두 번쨰 인자는 WSABUF 구조체 배열에 대한 포인터이다. 각 WSABUF구조체는 버퍼에 대한 포인터와  
길이를 담는다.  
세 번째 인자는 WSABUF 수이다.  
네 번쨰 인자는 전송된 바이터 수에 대한 포인터이다.  
다섯 번쨰 인자는 플래그인자이다.  
여섯 번쨰 인자는 WSAOVERLAPPED 개체에 대한 포인터이다. 오버랲드 함수 개시와 완료간의 매개체가 된다.

```c++
typedef struct _WSAOVERLAPPED {
  DWORD    Internal;
  DWORD    InternalHigh;
  DWORD    Offset;
  DWORD    OffsetHigh;
  WSAEVENT hEvent;
} WSAOVERLAPPED, *LPWSAOVERLAPPED;
```

일곱 번쨰 인자는 전송 완료시 실행할 함수에 대한 포인터이다.

에러 없이 전송을 완료하면 0을 실패하면 에러코드를 내뱉는다. WSA_IO_PENDING 에러는 작업이 시작은  
되었으나 아직 안 끝났다는 뜻이다.

오버랲드 동작 말고도 여러 버퍼를 한번에 모아 보낼 수 있다는 장점이 있다.  
5,6번 인자가 둘다 NULL이면 소켓은 non-overlapped 소켓으로 간주된다. 그 경우 send랑 같은 블로킹  
정책을 취한다.

이 함수는 다른 WSARecv, WSARecvFrom, WSASend, or WSASendTo 같은 오버랲드 함수의 완료 루틴안에  
포함 될 수 있다.

완료 루틴 인자가 널이면 완료시 오버랲드 개체의 이벤트가 시그널 상태가된다. 그게 아니라면 이벤트 인자는  
무시 되며 정보 전달용으로 쓸 수는 있다.

WSARecv도 사실상 같은 원리다.

### WSAGetOverlappedResult

지정 된 소켓에서 오버랲드 작업의 결과를 가져온다.  
첫 번쨰 인자는 오버랲드 동작을 하는 데에 쓰인 소켓 핸들이다.  
두 번쨰 인자는 오버랲드 함수 호출시에 지정된 WSAOVERLAPPED에 대한 포인터이다.  
세 번쨰 send/recv 연산을 통해 주고 받은 데이터에 대한 크기이다.  
네 번쨰 인자는 펜딩 상태에 있는 함수를 기다릴지에 대한 여부이다.  
다섯 번쨰 인자는 완료 상태에 대한 플래그이다.

반환값은 BOOL이다.

## Overlapped(이벤트 방식)

이제는 비동기이다 보니 동작에 연관 된 데이터들을 진짜로 완료할 때까지 보존 해야 한다.  
 WSACreateEvent()는 수동으로 설정함에도 불고하고 이벤트를 넌-시그널 상태로 안바꾸는 이유는  
 WSARecv같은 IO함수들은 오버랲드 개체의 이벤트를 넌-시그널로 상태로 바꾸고 시작하기 떄문이다.

```c++
Session session = Session{ clientSocket };
WSAEVENT wsaEvent = WSACreateEvent();
session.overlapped.hEvent = wsaEvent;

while (true)
{
	WSABUF wsaBuf;
	wsaBuf.buf = session.recvBuf;
	wsaBuf.len = 100;

	DWORD recvLen = 0;
	DWORD flags = 0;

	if (WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, nullptr) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSA_IO_PENDING)
		{
			WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
			WSAGetOverlappedResult(session.socket, &session.overlapped, &recvLen, FALSE, &flags);
			std::cout << recvLen << std::endl;
		}
	}
	else
	{
		std::cout << recvLen << std::endl;
	}
}
```

이 방식은 WSAWaitForMultipleEvents에서 한 번에 등록 가능한 이벤트 수의 제한으로 관리가 어렵다.

## Overlapped(콜백 방식)

이번에 볼 콜백 방식은 IOCP와 밀접한 관련이 있다. 근데 언제 이 콜백 함수를 부를 지가 문제다.  
다른 코드 실행중에 갑자기 끼어들면 문제가 생길 수 있기 때문이다. 그래서 비동기 IO함수를 호출  
한 스레드들을 AlertalbeWait 상태로 만들어야한다. 이 상태로 만들어 주는 함수는 다음과 같다.

- SleepEx
- WaitForSingleObjectEx
- WaitForMultipleObjectsEx
- SignalObjectAndWait
- MsgWaitForMultipleObjectsEx

콜백 함수와 함께 비동기 IO함수를 호출하면 커널은 쓰레드마다 존재하는 APC(Asynchronous Procedure Call)큐  
에다가 콜백 함수에 대한 포인터를 저장한다. 해당 스레드가 AlertalbeWait 상태가 되면 커널은 해당 스레드의  
APC큐를 체크하여 콜백 함수들이 있다면 가져와서 해당 스레드로 보낸다. 그리고 그 스레드는 함수를 실행한다.  
이 작업을 큐가 빌 때까지 반복한다.

만약 ACP큐가 비었는데 AlertalbeWait 상태가 되면 해당 스레드의는 다음의 조건들 중 하나가 만족되기 전까지  
멈춘다.

- 대기 중이던 커널 오브젝트가 시그널 상태가 됨
- 콜백 함수가 APC큐에 삽입됨

이 방식이 이벤트 방식보다 더 효율적이라고한다.

코드에서 바뀐 점은 콜백 함수 포인터를 전달 하고 AlertableWait상태로 바꾸기 위해 SleepEx()한 게 다다.  
오버랲드 인자는 당장 쓸일이 없어서 nullptr를 전달했다. 나중에 추가 정보 전달을 위한 인자로 활용할 수 있다.

```c++
	if (WSARecv(session.socket, &wsaBuf, 1, &recvLen, &flags,  nullptr, RecvCallback) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		if (error == WSA_IO_PENDING)
		{
			SleepEx(INFINITE,TRUE);
		}
	}
```

근데 콜백 함수도 정해진 시그내쳐를 따라야 한다.

```C++
void LpwsaoverlappedCompletionRoutine(
  DWORD dwError,
  DWORD cbTransferred,
  LPWSAOVERLAPPED lpOverlapped,
  DWORD dwFlags
)
{...}

void CALLBACK RecvCallback(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags)
{
	std::cout << recvLen<<std::endl;
}

```

첫 번쨰 인자는 IO완료 상태를 나타낸다.  
두 번쨰 인자는 이동된 바이트 수를 나타낸다.  
세 번쨰 인자는 비동기 IO함수를 호출 할떄 지정했던 오버랲드 구조체 포인터이다.  
네 번쨰 인자는 해당 콜과 연관된 플래그이다.

이 방식은 지원 안되는 비동기 소켓 함수가 몇몇 있다(accept). 또한 빈번한 AleratableWait이 발생한다.

## 리액터/프로액터 패턴

오늘 공부한 것과 관련된 디자인 패턴을 보고 끝내자. 근데 정보가 없어도 너무 없다.

### Reactor Pattern

리액터 패턴은 동시다발적 요청에 응답하기 위한 이벤트 핸들링 전략이다.  
리액터 패턴은 블로킹IO나 멀티스레딩 대신 이벤트에 기반하여 다수의 동시다발적인 IO작업을 최소의 지연으로  
처리 한다. 허나 콜백 방식이라서 디버깅이 어렵고 싱글스레드라서 처리량이 많이 요구되는 상황에서 무리다.

![리액터 패턴 클래스 다이어그램](https://upload.wikimedia.org/wikipedia/commons/thumb/2/2a/Reactor_Pattern_-_UML_2_Component_Diagram.svg/1280px-Reactor_Pattern_-_UML_2_Component_Diagram.svg.png)

![리액터 패턴 시퀀스 다이어그램](https://upload.wikimedia.org/wikipedia/commons/thumb/8/87/ReactorPattern_-_UML_2_Sequence_Diagram.svg/1280px-ReactorPattern_-_UML_2_Sequence_Diagram.svg.png)  
구성요소는 다음과 같다.

- Handle : IO와 데이터에 대한 요청에 대한 식별자. 보통 소켓이나 파일 디스크립터 또는 이와 같이 OS로부터  
  제공되는 것들이다.
- Demultiplexer : 핸들의 상태를 모니터링 하고 다른 서브시스템에 상태 변화를 알리는 이벤트 노티파이어.  
  select()함수가 대표적이다.
- Dispatcher : 리액티브 프로그램의 실제 이벤트 루프로, 이벤트 핸들러들을 관리하고, 이벤트 발생시 적절한  
  핸들러를 호출한다.

흐름은 다음과 같다.

1. 프로그램이 이벤트 핸들러를 디스패처에 등록한다.
2. 이벤트 루프를 실행하여 이벤트를 기다린다.
3. 핸들이 준비되면 디멀티플렉서가 디스패처에 이벤트를 알린다.
4. 디스패처는 적당한 핸들러에 핸들을 보낸다.
5. 이벤트를 처리한다.

~뒤 늦게라는 의미를 가지는 걸 생각하면 이해가 쉬울 것이다.

### Proactor Pattern

프로액터 패턴은 긴 작업들이 비동기로 동작하는 이벤트 핸들링 전략이다. 비동기 동작이 끝나면 완료 핸들러가  
호출 된다. 오버랲드나 IOCP가 대표적.

1. 프로액티브 개시자가 비동기 작업을 비동기 작업 처리자를 통해 시작하고 완료 핸들러를 정의한다.
2. 완료 핸들러는 작업 끝에 비동기 작업 처리자에서 호출 된다.

![프로액터 시퀀스 다이어그램](https://upload.wikimedia.org/wikipedia/commons/thumb/6/61/Proactor.VSD_SequenceDiagram.png/400px-Proactor.VSD_SequenceDiagram.png)

미리~ 라는 의미를 가지는 걸 생각하면 이해가 쉬울 것이다.

#### 참조

- [MSDN : WSAOVERLAPPED](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-wsaoverlapped)
- [MSDN : WSASend](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasend)
- [MSDN : WSARecv](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv)
- [MSDN : WSAGetOverlappedResult](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsagetoverlappedresult)
- [MSDN : Alertable I/O](https://learn.microsoft.com/en-us/windows/win32/fileio/alertable-i-o)
- [MSDN : LPWSAOVERLAPPED_COMPLETION_ROUTINE](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nc-winsock2-lpwsaoverlapped_completion_routine)
- [CppCon2017 리액터/프로액터 패턴](https://www.youtube.com/watch?v=iMRbm32O0ws)
- [리액터 패턴에 관한 글](https://www.modernescpp.com/index.php/reactor/)
- [Reactor pattern](https://en.wikipedia.org/wiki/Reactor_pattern)
- [](https://en.wikipedia.org/wiki/Proactor_pattern)
