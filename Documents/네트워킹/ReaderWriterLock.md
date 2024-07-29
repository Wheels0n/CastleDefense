# Reader-Write Lock

리더/라이터 락에 대해 알아보자

## Readers-Wrtiers Problem

R/W락은 Readers-Wrtiers문제들 중 하나를 푸는 방안 중하다. 이 문제는 동시성 문제의 예이다.  
적어도 이 문제의 세가지 버전이 있는 데, 같은 공유 자원을 여러 스레드에서 동시에 접근하려는  
상황을 다룬다.

스레드 하나가 공유 자원에 대해 쓰는 중에는 다른 스레드에서 읽기/쓰기가 제한되는 상황이 있다.  
(특히 동시에 공유자원을 수정하는 걸 막고, 두 개 이상의 reader들이 공유 자원을 동시에 접근 하고  
싶은 상황). 이런 경우 상호배제를 해도 되긴한다. 허나 한 스레드가 읽는 중이라고 다른 스레드가 읽  
기 동작을 하려고 대기 상태로 들어가는 건 바보같다. 읽기는 데이터 수정을 하지 않으니 안전하다.  
이것이 바로 첫번쨰 Readers-Wrtiers 문제의 동기로, 읽는 중에는 다른 읽기에 대한 배제가 없는 조건  
이 추가된다.

```c++
semaphore resource=1;
semaphore rmutex=1;
readcount=0;

/*
   resource.P() is equivalent to wait(resource)
   resource.V() is equivalent to signal(resource)
   rmutex.P() is equivalent to wait(rmutex)
   rmutex.V() is equivalent to signal(rmutex)
*/

writer() {
    resource.P();          //Lock the shared file for a writer

    <CRITICAL Section>
    // Writing is done

    <EXIT Section>
    resource.V();          //Release the shared file for use by other readers. Writers are allowed if there are no readers requesting it.
}

reader() {
    rmutex.P();           //Ensure that no other reader can execute the <Entry> section while you are in it
    <CRITICAL Section>
    readcount++;          //Indicate that you are a reader trying to enter the Critical Section
    if (readcount == 1)   //Checks if you are the first reader trying to enter CS
        resource.P();     //If you are the first reader, lock the resource from writers. Resource stays reserved for subsequent readers
    <EXIT CRITICAL Section>
    rmutex.V();           //Release

    // Do the Reading

    rmutex.P();           //Ensure that no other reader can execute the <Exit> section while you are in it
    <CRITICAL Section>
    readcount--;          //Indicate that you no longer need the shared resource. One fewer reader
    if (readcount == 0)   //Checks if you are the last (only) reader who is reading the shared file
        resource.V();     //If you are last reader, then you can unlock the resource. This makes it available to writers.
    <EXIT CRITICAL Section>
    rmutex.V();           //Release
}
```

위 솔루션의 경우, 첫번쨰 reader가 자원을 획득하면 다른 writer가 접근 못하게 잠근다. 그 후에오는  
reader들은 락 걸필요 없이 공유 자원에 접근한다. 다만 read참조수 부분에 대해서는 뮤텍스를 쓰고 있다.

강사는 w/r를 동시에 잡는 걸 허락하는 데다가 cas로 스핀락을 하고 있다. 일단은 자력으로 위 의사코드대로  
짜보겠다.

### 코드

```c++
enum class eLockFlag
{
	EMPTY = 0x0000'0000,
	READ  = 0x0000'FFFF,
	WRITE = 0xFFFF'0000
};

atomic<uint32_t> m_lockFlag;
```

read 카운트를 나타내는 변수와 Write중 인지나타내는 변수 대신에 정수하나로 나타내었다.

```c++
void Lock::WriteLock()
{
	// r/w 둘 다 없어야 함
	uint32_t expected = eLockFlag::EMPTY;
	uint32_t desired = eLockFlag::ON_WRITE;
	while (m_lockFlag.compare_exchange_strong(expected, desired)==false)
	{
		expected = eLockFlag::EMPTY;
	}

}

void Lock::WriteUnlock()
{
	m_lockFlag.store(eLockFlag::EMPTY);
}
```

쓰기 시에는 락을 소유한 사람이 없어야한다. 그런데 전에 보았듯이 그냥 무한루프를 하면 값 검사와  
설정으로 단계가 나누어져서 cas를 써줘야한다. 언락시에는 EMPTY으로 설정하면된다.

```c++
void Lock::ReadLock()
{
	uint32_t expected = m_lockFlag.load()&eLockFlag::ON_READ;
    uint32_t desired = expected+1;
    while (m_lockFlag.compare_exchange_strong(expected, desired)==false)
    {
        expected = m_lockFlag.load() & eLockFlag::ON_READ;
        desired = expected + 1;
    }
}

void Lock::ReadUnlock()
{
	m_lockFlag.fetch_sub(1);
}

```

읽기의 경우 똑같이 cas를 하되 실패시 expected값을 새로 load해줘야한다. 언락은 원자적  
연산을 통해 1을 뺸다.

### 기타 변경 사항

- 테스트를 위해 ThreadPool::DoTask()를 태스크 풀링직후 락해제하게 블록추가

#### 참조

- [위키 피디아 : Readers–writers problem](https://en.wikipedia.org/wiki/Readers%E2%80%93writers_problem)
