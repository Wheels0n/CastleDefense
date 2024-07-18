# WSAEventSelect함수

select 함수는 사용 후에 소켓 셋을 초기화/등록을 해줘야했다. 그래서 성능에 영향을 끼칠 수 밖에 없다.  
 동기방식이었던 select말고 비동기방식인 WSAEventSelect을 알아보자.

## 윈도우 이벤트 생성

이벤트 방식이니 당연히 그전에 이벤트를 생성해야 한다.

```c++
WSAEVENT listenEvent = WSACreateEvent();
```

성공하면 WSAEVENT(그냥 커널 핸들임)를, 실패시 WSA_INVALID_EVENT를 반환한다.  
이 이벤트는 수동으로 리셋하는 오브젝트로, 초기에는 신호가 설정되어 있지 않다.  
종료시에 WSACloseEvent를 해줘야한다. 이 함수는 성공여부를 부울로 반환한다.

```c++
for (int i = 0; i < sockets.size(); ++i)
{
    result = closesocket(sockets[i]);
    if (result == SOCKET_ERROR)
    {
        std::cout << "closesocket() Failed" << std::endl;
    }

    WSACloseEvent(events[i]);
}
```

## WSAEventSelect함수

이 함수는 임의의 소켓을 대상으로 이벤트 발생여부를 관찰 한다.

첫번쨰 인자는 소켓이다.  
두번쨰 인자는 이벤트 개체에 대한 핸들이다.  
세번쨰 인자는 관찰하고자 하는 이벤트의 조합을 나타내는 비트마스크이다.  
관찰 가능한 이벤트 종류는 다음과 같다.

- FD_READ
- FD_WRITE
- FD_OOB(아웃 오브 밴드 데이터 수신 여부)
- FD_ACCEPT
- FD_CLOSE
- FD_CONNECT
- FD_OOB

성공하면 0을, 실패하면 SOCKET_ERROR를 반환한다. 이벤트 발생과는 무관하게 바로  
반환한다.

```c++
std::vector<SOCKET> clientSockets;
std::vector<WSAEVENT> events;

WSAEVENT listenEvent = WSACreateEvent();
events.push_back(listenEvent);

result = WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT);
if (result != 0)
{
	std::cout << "wsaselect to accept failed()" << std::endl;
	int error = WSAGetLastError();
	if (error != WSAEWOULDBLOCK)
	{
		std::cout << "ErrorCode: " << error << std::endl;
	}
}
```

성공적으로 기록을 하고 이벤트에 신호를 주었다면 해당 이벤트에 대해서는 앱에서 따로  
해당 이벤트에 대한 세팅과 신호주기를 재활성화하는 함수를 호출하지 않는 이상 추가  
동작은 행해지지 않는다. 재활성화 루틴에 대한 호출은, 실패를 하더라도, 기록과 신호  
주기의 재활성화로 이어진다.

FD_READ, FD_OOB, 그리고 FD_ACCEPT 이벤트의 경우 이벤트 기록과 신호주기가 레벨 트리거 방식  
이다. 즉, 재활성화 루틴 호출되고 관련 조건이 유효하다면 해당 이벤트는 기록되고 활성화된다.  
예를 들어

1. 100바이트의 데이터를 받아서 FD_READ가 설정되었다.
2. recv로 50바이트만 읽는다.
3. 여전히 읽을 데이터가 있으니 FD_READ이벤트가 또 설정된다.

- 에지 트리거 : 신호 상태의 변화를 인지하는 데에 쓰인다.
- 레벨 트리거 : 특정 시그널 상태를 인지하는 데에 쓰인다.

만약 어떠한 네트워크 동작이 이 함수 또는 재활성화 함수를 부르기 전에 발생하면 이벤트는 기록되고  
설정된다.

FD_WRITE는 조금 다르게 처리 되는 데, 소켓이 connect함수를 통해 연결되거나 accept함수를 통해  
 받아들여진 그후 send에는 WSAEWOULDBLOCK로 실패하고 버퍼공간이 여유가 생긴 경우다.

이 함수를 호출하면 소켓이 자동으로 넌블락모드가 되니 주의 해야한다. 또한 호출시 이전에 호출 된 것 취소하고  
해당 이벤트도 초기화 해버리니 주의한다.

accept함수로 생성된 소켓은 리스닝 소켓과 같은 속성을 가지니 다른 이벤트 핸들이나 관찰 이벤트를 원한다면  
따로 WSAEventSelect함수를 호출해줘야한다.

발생여부는 WSAWaitForMultipleEvents함수로 확인하고, 이벤트에 대한 정보는 WSAEnumNetworkEvents함수로 확인한다.

select처럼 리셋은 안해줘도 되지만 여전히 이 방식도 WSAWaitForMultipleEvents에 등록가능한 이벤트 수의 제한떄문에  
한번에 처리 가능하는 데에 한도가 있다. 또한 클라이언트마다 이벤트를 관리하기도 힘들다.

## 발생 여부 확인하기

WSAWaitForMultipleEvents함수는 지정된 이벤트 개체 중 하나 또는 전부가 신호를 받았거나, 시간제한에 걸리거나,  
IO완료 루틴이 실행된 경우 반환한다. 그전까지는 프로세서 시간을 사용하지 않고 해당 스레드는 대기 상태가 된다.

첫번쨰 인자는 이벤트 핸들의 수이다.  
두번쨰 인자는 이벤트 배열에 대한 포인터이다.  
세번쨰 인자는 모든 이벤트를 기다릴지에 대한 부울이다.  
네번쨰 인자는 제한시간이다. 0이면 즉시 반환한다.  
다섯번쨰 인자는 시스템 I/O완료 루틴을 실행할 수 있게 스레드가 경고 가능한 대기 상태에  
배치 되는 지 여부를 지정한다. 지금은 사용 안할 거다..

성공시에는 성공한 이벤트의 인덱스, 실패시 WSA_WAIT_FAILED를 반환한다.

```c++
std::vector<Session> sessions;
while (true)
{

	int idx = WSAWaitForMultipleEvents(events.size(), &events[0], FALSE, INFINITE, FALSE);
	if (idx == WSA_WAIT_FAILED)
	{
		int error = WSAGetLastError();
		std::cout << "ErrorCode: " << error << std::endl;
		break;
	}
	else
    {
        //...
    }
}
```

## 발생 정보 가져오기

WSAEnumNetworkEvents함수는 마지막 호출이후 해당 소켓에 발생한 이벤트 검색하는 데 쓰인다.

첫번쨰 인자는 소켓이다.  
두번쨰 인자는 선택사항으로, 재활성화할 이벤트 핸들이다.  
세번쨰 인자는 WSANETWORKEVENTS포인터로, 발생한 에러를 출력하는 인자다.  
에러 코드는 FD_WRITE_BIT같은 식별자로 에러코드 배열에 접근 할 수 있다.

```c++
typedef struct _WSANETWORKEVENTS {
       long lNetworkEvents;
       int iErrorCode[FD_MAX_EVENTS];
} WSANETWORKEVENTS, FAR * LPWSANETWORKEVENTS;

```

성공시 0을, 실패시 SOCKET_ERROR를 반환한다.

```c++
else
{
	WSANETWORKEVENTS networkEvents;
	result = WSAEnumNetworkEvents(sockets[idx], events[idx], &networkEvents);
	if (result == SOCKET_ERROR)
	{
		//에러처리

	}

	if (networkEvents.lNetworkEvents & FD_ACCEPT && networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
	{
        //Accept호출

		WSAEVENT clientEvent = WSACreateEvent();
		sockets.push_back(clientSocket);
		events.push_back(clientEvent);

		std::string idxStr = std::to_string(sessions.size());
		Session session;
		strcpy(session.sendBuf,idxStr.c_str());
        sessions.push_back(session);
		result = WSAEventSelect(clientSocket, clientEvent, FD_READ| FD_WRITE);
		if (result != 0)
		{
			int error = WSAGetLastError();
			std::cout << "ErrorCode: " << error << std::endl;
		}
	}
    //클라이언트는 Send-Recv순서를 지키는데 FD_READ만 켜진경우 읽기만 하면 읽기 이후에 어떠한 플래그도 안켜져서
    //먹통이 될 수있다.
		if ((networkEvents.lNetworkEvents & (FD_READ)) && (networkEvents.iErrorCode[FD_READ_BIT] == 0)
		|| ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] == 0)))
	{
		Session& session = sessions[idx - 1];
		result = recv(sockets[idx], session.recvBuf, 100, 0);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			std::cout << "Recv ErrorCode: " << error << std::endl;
			break;

		}
		std::cout << session.recvBuf << std::endl;
		std::cout << "Received : " << sizeof(session.recvBuf) << std::endl;

		result = send(sockets[idx], session.sendBuf, 100, 0);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			std::cout << "Send ErrorCode: " << error << std::endl;
			break;
		}

		std::cout << "Send Data" << std::endl;
	}

}
}
```

### 기타 변경사항

- 더미별로 따로 유저 입출력버퍼를 가지도록 Session 구조체 생성

#### 참조

- [MSDN : WSAEventSelect](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsaeventselect)
- [Edge Triggering and Level Triggering](https://www.geeksforgeeks.org/edge-triggering-and-level-triggering/)
- [MSDN : WSACreateEvent](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsacreateevent)
- [MSDN : WSACloseEvent](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsacloseevent)
- [MSDN : WSAWaitForMultipleEvents](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsawaitformultipleevents)
- [MSDN : WSAEnumNetworkEvents](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-wsaenumnetworkevents)
