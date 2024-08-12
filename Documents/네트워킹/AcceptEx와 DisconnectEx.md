# AcceptEx와 DisconnectEx

미뤄 두었던 AcceptEx를 알아보자.

## AcceptEx

AcceptEx는 accept의 오버랲드(비동기)버전이다. 미리 클라이언트 용 소켓을 따로 준비해둬야한다.  
대신에 로컬/리모트 주소를 반환하고 클라이언트에서 온 첫 데이터 블록을 받는다.

첫 번쨰 인자는 리슨 소켓이다.  
두 번쨰 인자는 들어올 연결을 받을 소켓이다.  
세 번쨰 인자는 새 연결을 통해 받은 첫 데이터 블록을 받을 버퍼에 대한 포인터이다.  
네 번쨰 인자는 버퍼 시작 부분에 받을 데이터의 크기이다.  
다섯 번쨰 인자는 로컬 주소를 위해 예약된 바이트 크기이다.  
여섯 번쨰 인자는 리모트 주소를 위해 예약된 바이트 크기이다.  
일곱 번째 인자는 전송 받은 바이트 수를 받는 DWORD에 대한 포인터이다.  
여덟 번쨰 인자는 오버랲드 구조체에대한 포인터이다.

성공 여부를 BOOL로 반환한다.

AcceptEx는 여러 소켓 함수를 하나로 친 것과 같다. 성공시 행하는 동작은 다음과 같다.

- 새 연결을 받는다.
- 로컬/리모트 주소를 반환한다.
- 리모트로부터 데이터의 첫 블록을 받는다.

하나의 출력 버퍼로 데이터, 로컬 주소(서버), 리모트 주소(클라이언트)를 받는다.  
만약 버퍼가 명시되면 연결되고 데이터를 읽기까지 해야지 반환한다. 주소 버퍼 크기는  
내부 형식으로 작성되서 사용 중인 전송 프로토콜의 sockaddr 구조 크기보다 16바이트  
더 커야한다.

반환하면 클라이언트 소켓은 기본 상태로 있고 리슨 소켓의 속성을 상속 받지 않는다.

SO_UPDATE_ACCEPT_CONTEXT로 accept후 쓸 소켓의 속성을 설정해야지 getsockopt()가  
제대로 동작한다. 리슨 소켓으로부터 상속된 속성들을 갱신한다.

## ConnectEx

당장 쓸 일은 없지만 connect()의 비동기 버전도 보자. lpfnConnectex()라고 이름이 되어있다.  
오로지 연결 지향형 소켓에만 사용 가능하다.

첫 번쨰 인자는 연결을 할 소켓이다.  
두 번쨰 인자는 sockaddr에 대한 포인터이다.  
세 번쨰 인자는 sockaddr 구조체의 바이트 크기이다.  
네 번쨰 인자는 연결 후 보낼 데이터를 담는 버퍼에 대한 포인터이다.  
다섯 번쨰 인자는 버퍼에 담긴 데이터 크기이다.  
여섯 번쨰 인자는 전송 된 바이트의 크기를 나타내는 DWORD에 대한 포인터이다.  
일곱 번쨰 인자는 OVERLAPPED 구조체에 대한 포인터이다.

성공 여부를 BOOL로 반환한다. 성공하면 소켓은 기본 상태로 존재한다.

이 함수도 여러 소켓 함수를 하나로 합친 것과 같다.

- 새 연결을 한다.
- (있다면)데이터 블록을 보낸다. 마치 send/WSASend처럼 쓸 수는 있다.  
  다만 시스템 자원을 많이 먹는 다고 한다.

## DisconnectEx

몰랐는 데 이런 것도 있다. 소켓의 연결을 끊고 다시 쓸 수 있게 한다.  
오로지 연결 지향형 소켓에만 사용 가능하다.

첫 번쨰 인자는 연결된 소켓이다  
두 번쨰 인자는 OVERLAPPED구조체에 대한 포인터이다.  
세 번쨰 인자는 플래그이다.  
네 번쨰 인자는 0으로, 예약이 되어있다.

성공여부를 BOOL로 반환한다.

연결 종료(closure)와 연결 해제(release) 사이의 간격을 TIME_WAIT라고 한다. 이 간격에는  
연결 자체를 다시 하는 것 보다 더 적은 비용으로 다시 열 수 있다. 만약 연결이 해제 되면  
소켓과 내부 자원은 다른 연결을 위해 쓰일 수 있다.

윈도우 TCP도 연결이 종료 된후 TIME_WAIT상태에 들어 가는 데, 이 간격에는 소켓과 자원이  
재활용 될 수 없다.

HKEY_LOCAL_MACHINE\System\CurrentControlSet\Services\TCPIP\Parameters\TcpTimedWaitDelay  
를 통해 간격을 설정 할 수 있다.

## WSAIoctl

근데 위의 함수들은 런타임에 WSAIoctl를 호출해서 함수 포인터들을 가져와야한다.  
이 함수 주 목적은 소켓, 전송 프로토콜, 또는 통신 스템에 대한 인자들을 가져오거나 설정하는 것이다.

첫 번쨰 인자는 소켓이다.  
두 번쨰 인자는 수행 할 동작에 대한 코드이다.  
세 번쨰 인자는 입력 버퍼에 대한 포인터이다.  
네 번쨰 인자는 입력 버퍼의 크기이다.  
다섯 번쨰 인자는 출력 버퍼에 대한 포인터이다.  
여섯 번쨰 인자는 출력 버퍼의 크기이다.  
일곱 번쨰 인자는 실제 출력의 바이트 크기에 대한 포인터이다.  
여덟 번쨰 인자는 WSAOVERLAPPED 구조체에 대한 포인터이다.  
아홉 번쨰 인자는 IOCP에 대한 콜백 함수에 대한 포인터이다.

성공 하면 0을, 실패하면 SOCKET_ERROR를 반환한다.

만약 lpOverlapped 와 lpCompletionRoutine 모두 널이라면 소켓은 넌 오버랲드 소켓으로 간주 된다.  
그렇게 되면 ioctlsocket 처럼 동작한다.

SIO_GET_EXTENSION_FUNCTION_POINTER를 opcode로 하고 WSAID_ACCEPTEX를 입력버퍼에 넣는다.

```c++
// Load the AcceptEx function into memory using WSAIoctl.
    // The WSAIoctl function is an extension of the ioctlsocket()
    // function that can use overlapped I/O. The function's 3rd
    // through 6th parameters are input and output buffers where
    // we pass the pointer to our AcceptEx function. This is used
    // so that we can call the AcceptEx function directly, rather
    // than refer to the Mswsock.lib library.
    iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
             &GuidAcceptEx, sizeof (GuidAcceptEx),
             &lpfnAcceptEx, sizeof (lpfnAcceptEx),
             &dwBytes, NULL, NULL);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"WSAIoctl failed with error: %u\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
```

#### 참조

- [MSDN : AcceptEx](https://learn.microsoft.com/ko-kr/windows/win32/api/mswsock/nf-mswsock-acceptex)
- [MSDN : ConnectEx](https://learn.microsoft.com/en-us/windows/win32/api/mswsock/nc-mswsock-lpfn_connectex)
- [MSDN : WSAIoctl](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsaioctl)
- [MSDN : DisconnectEx](<https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms737757(v=vs.85)>)
- [MSDN : SOL_SOCKET Socket Options](https://learn.microsoft.com/en-us/windows/win32/winsock/sol-socket-socket-options)
