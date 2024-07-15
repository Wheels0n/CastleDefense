# select함수

넌블로킹 소켓들의 수가 많은 상황에서 이들의 호출이 제대로 성공 했는 지  
하기 위해 루프문을 돌면 부담이 많이 된다. 이를 해결하기 위한 첫번쨰 방법  
으로 select함수가 있다. 우리는 물론 iocp를 쓸 것이지만 클라이언트는 1:1  
통신이여서 그렇게 고급기법까지 갈 필요는 없다.

## select함수

위 문제점에 대한 대안으로 소켓 라이브러리에는 여러 소켓을 한번에 확인하고  
그 중에 하나라도 준비되면 즉시 대응할 수 있는 방법이 마련되어 있다.

우리가 원하는 것은

1. 소켓리스트를 받는다.
2. 소켓 중 하나라도 IO처리가 가능할 때까지 블로킹 한다.
3. 블로킹이 끝나면 어떤 소켓이 IO처리가 가능한 지 알려준다.

이러한 작업을 가능케 하는 것이 바로 select이다. 여기서 IO처리가 가능하다는 것은  
호출시 WOULDBLOCK이 아닌 다른 결과가 나온다는 뜻이다. 예를들어 송수신함수의 경우  
버퍼에 1바이트의 공간이 남았거나, 1바이트의 데이터가 있다면 처리가능하다.

첫번쨰 인자는 nfds로 POSIX플랫폼에서 소켓 식별자 용으로 쓴다. 근데 윈도우에서는 그냥  
정수가 아니라 포인터라서 무시된다.  
두번쨰 인자는 가독성을 확인할 소켓집합에 대한 포인터이다.  
세번쨰 인자는 쓰기 가능성을 확인할 소켓집합에 대한 포인터이다.  
네번쨰 인자는 오류를 확인할 소켓집합에 대한 포인터이다.  
다섯번쨰 인자는 블로킹 시간제한이다.

반환시 가독성, 쓰기 가능성, 오류 검출이 된 모든 소켓의 총합을, 시간초과나 오류 발생시에
0을 반환한다. 또한 통과 못한 소켓은 소켓 그룹에서 제거된다.

fd_set인자에 관한 작업을 편리하게 해주는 마크로들이 있다.

- FD_ZERO(\*fd_set) : 소켓 그룹 비우기
- FD_SET(s, \*fd_set): 소켓 그룹에 소켓 넣기
- FD_CLR(s, \*fd_set): 소켓을 소켓 그룹에서 제거
- FD_ISSET(s, \*fd_set): 소켓 그룹에 소켓이 포함여부 확인

소켓이 가독성이 있다는 것은 recv가능하다는 것 말고도 소켓이 리슨 상태에 있어서 accept가능한  
것도 포함한다. 또한 연결 지향 소켓에 한해서 소켓을 닫으려는 요청 수신됭 경우도 포함된다.

```c++
result = select(0, &reads, nullptr, nullptr, nullptr);
if (result==0)
{
	//에러처리
}
clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
```

쓰기 가능성은 send가능성 말고도 넌블락 소켓의 connect함수가 성공 되었는 지 확인하기
위해 쓸 수도 있다.

서버만 select방식으로 바꾸었다.

```c++
FD_SET(clientSocket, &reads);
FD_SET(clientSocket, &writes);
cpyReads = reads;
cpyWrites = writes;
while (true)
{
	reads = cpyReads;
	writes = cpyWrites;

	char sendBuf[100] = "received";
	char recvBuf[100];

	result = select(0, &reads, nullptr, nullptr, nullptr);

	if(result)
	{
		result = recv(clientSocket, recvBuf, 100, 0);
		if (result == SOCKET_ERROR)
		{
			//에러처리
		}
		else
		{
			std::cout << recvBuf << std::endl;
			std::cout << "Received : " << sizeof(recvBuf) << std::endl;

			result = select(0, nullptr, &writes, nullptr, nullptr);
			if(result)
			{
				result = send(clientSocket, sendBuf, 100, 0);
				if (result == SOCKET_ERROR)
				{
					//에러처리

				}
                std::cout << "Send Data" << std::endl;
			}
			else
			{
				//에러처리
			}
		}

	}
	else
	{
		//에러처리
	}
}
```

허나 매번 반복 등록도 귀찮을 뿐더러 FD_SETSIZE한번에 세팅가능한 크기가 생각보다 작다.  
동접자가 많으면 FD_SET의 묶음이 필요해진다.

#### 참조

- [MSDN : select](https://learn.microsoft.com/ko-kr/windows/win32/api/winsock2/nf-winsock2-select)
