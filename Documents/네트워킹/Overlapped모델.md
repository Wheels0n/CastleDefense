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

#### 참조

- [MSDN : WSAOVERLAPPED](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/ns-winsock2-wsaoverlapped)
- [MSDN : WSASend](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsasend)
- [MSDN : WSARecv](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsarecv)
- [MSDN : WSAGetOverlappedResult](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsagetoverlappedresult)
