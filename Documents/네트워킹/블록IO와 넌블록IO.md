# 블락IO와 넌블록IO

지금까지 MSDN에서 살펴보고 넘긴 블락방식과 넌블럭 방식에 대해 알아보자.

## 블로킹 IO

소켓함수 대부분은 블로킹 호출이다. connect, accept, send, sendto, recv, recvfrom  
등, 완료시까지 해당 스레드가 대기상태로 빠지기떄문에 문제가 된다. 접속자가 수 천명인데  
이 문제를 해결할 방법으로 멀티스레딩, 넌블로킹IO가있다.

멀티스레딩 방식은 클라이언트마다 새로운 스레드를 할당하여 처리하는 방법이다. 허나 이방식은  
접속자 수가 많아지면 컨텍스트 스위칭으로인한 오버헤드가 심해진다.

## 넌브로킹 IO

소켓은 기본 설정상 블로킹모드로 동작한다. 하지만 넌블로킹모드도 지원한다. 넌블로킹 소켓은 함수  
호출 후 즉시 -1(WSAEWOULDBLOCK, 원래라면 블로킹되었어야했다는 뜻일뿐 에러는 아님)을 반환한다.

윈도우에서 소켓을 넌블로킹모드로 전환하려면 ioctlsocket함수를 호출하면 된다.

첫번쨰 인자는 소켓이다.  
두번쨰 인자는 수행할 명령이다.  
세번쨰 인자는 명령 인자에대한 포인터이다.  
성공하면 0을, 실패시 SOCKET_ERROR를 반환한다.

FIONBIO가 블록/넌블록 모드에 관한 설정이며 설정값이 0이면 블록, 그외는 넌블럭이다.

```c++
u_long mode = 1;
result = ioctlsocket(hListenSocket, FIONBIO, &mode);
```

그리고 이제는 INVALID_SOCKET이라고 무조건 진짜 오류가 아니고 WSAEWOULDBLOCK인지 확인을 해줘야한다.  
다만 connect의 경우 WSAEWOULDBLOCK의 의미가 조금 다른다. 다른 송수신 함수는 아무 일도 안 일어 났다는  
뜻으로 이 값을 반환하지만 connect는 이미 뭔가 일어 났다는 걸 나타낸다.

그렇다면 성공하거나 에러 발생시까지 반복해야한다. 나머지 블록 함수들도 같은 방식으로 처리해 줘야한다.

```c++
while (true)
{
	sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(sockaddr_in));
	int addrLen = sizeof(sockaddr_in);
	SOCKET clientSocket = accept(hListenSocket, (sockaddr*)&clientAddr, &addrLen);
	if (clientSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			std::cout << "Accept ErrorCode: " << error << std::endl;
			return -1;
		}
	}
}
```

근데 이런식으로 루프문을 계속돌리면 부담이 된다. 더군다나 소켓도 많을 텐데 하나하나 이런 작업을 한다?  
무리다.

### 기타 변경사항

- 다시 TCP 방식으로 전환

#### 참조

- [MSDN : ioctlsocket](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock/nf-winsock-ioctlsocket)
- [MSDN : connect](https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect)
