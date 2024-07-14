# 소켓 입출력함수(UDP)

어차피 TCP를 쓸 것이긴 하다만 일단 공부목적으로 가볍게 보고 넘기자.

## TCP와 비교

### 초기화

윈속 초기화 부분은 당연히 같다.

```c++
WSADATA wsaData;
int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
if (result != 0)
{
	std::cout << "WSAStartup() Failed" << std::endl;
	return -1;
}
```

### 소켓 생성

type인자로 SOCK_DGRAM을 넘겨줘야 UDP 소켓이된다.

```c++
SOCKET  hSocket = socket(PF_INET, SOCK_DGRAM, 0);
if (hSocket == INVALID_SOCKET)
{
	std::cout << "socket() Failed" << std::endl;
	return -1;
}
```

### 주소 바인딩

역시나 달라진 건 없다.

```c++
sockaddr_in serverAddr;
memset(&serverAddr, 0, sizeof(sockaddr_in));
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(777);
InetPton(AF_INET, L"127.0.0.1", &serverAddr.sin_addr);

result = bind(hSocket, (sockaddr*)&serverAddr, sizeof(sockaddr_in));
if (result == SOCKET_ERROR)
{
	std::cout << "bind() Failed" << std::endl;
	return -1;
}

```

### 바로 입출력

이제 바로 입출력으로 넘어간다. 리슨 - 어셉트따위 필요없다. 다만 사용 함수가 달라 진다.
recvfrom()이라는 함수를 써야한다. 인자는 recv에서 2개 더 추가 되었다.

첫번쨰 인자는 바인딩 된 소켓이다.  
두번쨰 인자는 수신된 데이터를 담는 버퍼이다.  
세번쨰 인자는 수신버퍼의 크기이다.  
네번째 인자는 플래그이다.  
다섯번쨰 인자는 주소 데이터를 받을 구조체 포인터이다.  
여섯번쨰 인자는 주소 데이터의 크기에대한 포인터이다.

accept로 처리하던게 합쳐진 느낌이다.

성공하면 받은 바이트 수를, 실패하면 SOCKET_ERROR반환한다.

연결된 소켓과 연결되지 않은 소켓 모두에서 들어오는 데이터를 읽고 출처되는 주소를 캡처한다.  
보통은 연결 없는 소켓과 사용된다. SOCKET_STREAM으로 된 소켓에서 부르면 지정된 버퍼크기까지,  
현재 사용 가능한 정보를 반환한다.

소켓은 자신의 주소를 알고 있어야하는 데 보통 서버측에서는 bind()를 쓰고, 클라측에서는 그냥  
sendTo()같은 함수를 통해 암시적으로 바인딩 될수 있다.

```c++
sockaddr_in clientAddr;
memset(&clientAddr, 0, sizeof(sockaddr_in));
char sendBuf[100]="received";
char recvBuf[100];
int fromLen = sizeof(sockaddr_in);
result = recvfrom(hSocket, recvBuf, 100, 0, (sockaddr*)&serverAddr, &fromLen);
if (result == SOCKET_ERROR)
{
	int error = WSAGetLastError();
	std::cout << "Recv ErrorCode: " << error << std::endl;
}
```

전송의 경우도 다른 함수가 있다. sendto라는 함수를 쓴다. 인자는 recvfrom 매우 비슷하다.

첫번쨰 인자는 바인딩 된 소켓이다.  
두번쨰 인자는 송신할 데이터를 담는 버퍼이다.  
세번쨰 인자는 송신할 데이터 크기이다.  
네번째 인자는 플래그이다.  
다섯번쨰 인자는 대상 주소 데이터를 받을 구조체 포인터이다.  
여섯번쨰 인자는 대상 주소 데이터의 크기이다.

성공하면 전송한 바이트 수를(len보다 더 작을 수 있다), 실패하면 SOCKET_ERROR반환한다.  
성공적으로 반환했따고 꼭 데이터가 성공적으로 전달되었다는 걸 뜯하지 않는다.

소켓이 바인딩 되지 않은 경우 고유값이 시스템에 의해 할당되어 바인딩 된 것으로 처리된다.

```c++
sendto(hSocket, sendBuf, 100, 0, (sockaddr*)&clientAddr, fromLen);
if (result == SOCKET_ERROR)
{
	int error = WSAGetLastError();
	std::cout << "Recv ErrorCode: " << error << std::endl;
}
```

클라쪽에서도 connect는 날려버리고 대칭적으로 해주면 끝이난다.

#### 참조

- [MSDN : recvfrom](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-recvfrom)
- [MSDN : sendto](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-sendto)
