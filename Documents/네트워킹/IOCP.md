# IOCP

드디어 대망의 IOCP다. 절반은 왔다. 오버랲드를 이해했다면 크게 어렵진 않다.

## I/O Completion Ports

IOCP는 멀티프로세서 시스템에서 다수의 비동기 IO요청을 처리하는 데 효과적인 쓰레딩 모델을 제공한다.  
프로세스에서 IOCP를 만들면 시스템은 워커스레드들은 위한 큐 개체를 만든다.

### 어떻게 동작 하는 가

CreateIoCompletionPort()로 IOCP를 만들고 해당 포트에 하나 이상의 핸들을 등록한다.

첫 번쨰 인자는 핸들이다. INVALID_HANDLE_VALUE이면 IOCP를 만들고, 유효한 핸들이면 ICOP에 등록한다.  
두 번쨰 인자는 IOCP에 대한 핸들이다. NULL이면 IOCP를 만들고, 유효하면면 핸들을 이 ICOP에 등록한다.  
세 번쨰 인자는 해당 핸들에 대한 ICOP 패킷에 포함되는 유저 정의 핸들별 완료 값이다. unqiue해야한다.  
네 번쨰 인자는 OS에서 해당 IOCP에 대한 IO완료 패킷을 동시에 처리할 최대 스레드수이다.

성공하면 IOCP에 대한 핸들을, 실패하면 NULL을 반환한다.

IO시스템은 IO완료 패킷이 큐되는 IOCP에다가 패킷을 보낼 수 있다. 이게 바로 이 함수의 목적이다.  
IOCP핸들은 한 프로세스 내의 스레드끼리는 공유가 가능하다. 파일핸들(오버랲드 지원하면 다 됨)은  
한번에 한 IOCP에만 등록된다.

등록 된 핸들의 비동기 IO작업이 끝나면 등록된 IOCP에 FIFO방식으로 패킷이 등록된다.  
IOCP에 핸들이 등록되면 상태 블록은 IOCP로부터 패킷이 제거 되기 전까지 갱신 되지 않는다. 스레드는  
GetQueuedCompletionStatus()를 이용해서 IOCP에 완료 패킷이 도착할때까지 기다릴 수 있다. 이 함수는  
IO완료 패킷을 IOCP로부터 dequeue를 시도한다. 만약 완료 패킷이 없다면 보류 중인 IO작업 완료 될 때까지  
대기한다.

첫 번쨰 인자는 IOCP에 대한 핸들이다.  
두 번쨰 인자는 완료된 IO 작업을 통해 전송된 패킷의 양을 나타내는 변수에 대한 포인터이다.  
세 번쨰 인자는 CreateIoCompletionPort() 당시 등록된 완료 키이다.  
네 번쨰 인자는 IO함수 호출 시에 등록된 오버랲드 구조체에 대한 포인터이다.
다섯 번쨰 인자는 대기시간이다.

성공하면 0이 아닌 값을, 실패하면 0을 반환한다.

IOCP에 실행을 블락하는 스레드들은 LIFO방식으로 해제되고 다음 완료 패킷은 IOCP의 FIFO큐로부터 가져온다.  
즉, 완료 패킷이 어느 스레드에 전달 될 떄, 시스템은 가장 최근의 스레드에다가 가장 오래된 작업의 완료 정보  
를 전달한다.

스레드는 한 번에 하나의 IOCP에 등록 된다. 스레드들은 PostQueuedCompletionStatus()를 통해 완료 패킷을  
삽입할 수 있다. 이를 통해 다른 프로세스의 스레드들과 통신 가능하다.

## 코드

Accept도 비동기 버전이 있지만 복잡해서 당장은 accept는 메인스레드에서 하고, 나머지 스레드에서 send/recv
할 것이다.

리슨 소켓을 만든뒤 iocp를 만든다. 그후 바로 워커 스레드를 실행한다. 워커 스레드는 읽기 완료가 되기 전까지  
블록 된다. 송신후 완료 키 인자로 받은 세션 포인터와 오버랲드 인자로 다시 WSARecv하게 한다. 오버랲드 인자를  
세션에서 분리한 이유는 나중에 하나의 소켓으로 연달아 호출할 수 도 있기 떄문이다.

```C++
HANDLE hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

ThreadPool* pThreadPool = new ThreadPool();
for (int i = 0; i < 5; ++i)
{
	pThreadPool->EnqueueTask([=]() { WorkerThreadMain(hIocp); });
}


void WorkerThreadMain(HANDLE hIocp)
{
	while (true)
	{
		DWORD transferredBytes=0;
		Session* pSession = nullptr;
		OverlappedEx* pOverlappedEx = nullptr;

		BOOL ret = ::GetQueuedCompletionStatus(hIocp, &transferredBytes,
			(ULONG_PTR*)&pSession, (LPOVERLAPPED*)&pOverlappedEx, INFINITE);

		if (ret == FALSE || transferredBytes ==0)
		{
			continue;
		}

		std::cout << transferredBytes << std::endl;

		WSABUF wsaBuf;
		wsaBuf.buf = pSession->recvBuf;
		wsaBuf.len = 100;

		DWORD recvLen = 0;
		DWORD flags = 0;

		WSARecv(pSession->socket, &wsaBuf, 1, &recvLen, &flags, &pOverlappedEx->overlapped, NULL);
	}
}

```

메인 스레드에서는 들어오는 connect요청을 받고 바로 WsaRecv한다.

```c++
	std::vector<Session*> sessionManager;

	while (true)
	{
		SOCKADDR_IN clientAddr;
		int addrLen = sizeof(clientAddr);
		SOCKET clientSocket = INVALID_SOCKET;

		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			return -1;
		}

		Session* pSession = xnew<Session>();
		pSession->socket = clientSocket;
		sessionManager.push_back(pSession);

		std::cout << "Client Connected!" << std::endl;
		CreateIoCompletionPort((HANDLE)clientSocket, hIocp, (ULONG_PTR)pSession, 0);


		WSABUF wsaBuf;
		wsaBuf.buf = pSession->recvBuf;
		wsaBuf.len = 100;

		OverlappedEx* pOverlappedEx = xnew<OverlappedEx>();
		pOverlappedEx->ioType = READ;

		DWORD recvLen = 0;
		DWORD flags = 0;

		WSARecv(pSession->socket, &wsaBuf, 1, &recvLen, &flags, &pOverlappedEx->overlapped, NULL);

	}
```

### 기타 변경 사항

- 서버코어 프로젝트르를 정적 LIB으로 빌드 후 사용

#### 참조

- [MSDN : I/O Completion Ports](https://learn.microsoft.com/en-us/windows/win32/fileio/i-o-completion-ports)
- [MSDN : CreateIoCompletionPort](https://learn.microsoft.com/en-us/windows/win32/fileio/createiocompletionport)
- [MSDN : GetQueuedCompletionStatus](https://learn.microsoft.com/en-us/windows/win32/api/ioapiset/nf-ioapiset-getqueuedcompletionstatus)
