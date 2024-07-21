# future

단발성 이벤트로, 조건변수 대용으로 쓸만한 std::future를 알아보자.  
뮤텍스나 조건변수보다는 쓰는 경우가 적긴하다.

## std::future

메인함수에서 어떤 함수를 호출한다고 치자. 동기방식이라 반환시까지 메인 함수는  
흐름이 멈춘다. 만약 그 함수가 중요도가 떨어지는 데 오래 걸린다면 비동기방식으로  
다른 스레드에 넘기거나 나중에 실행하는 게 나을 것이다. 근데 그러면 단점이 있다.

- 데이터를 넘기기 위해 공용 데이터를 쓴 다는 점
- 추가적인 동기화 작업이 필요

일 한번 시킬 건데 그렇게까지 해야하나 싶다. 그래서 std::future를 쓸건데 생성 방법은  
여러가지가 있는데 std::async를 쓸 것이다. 인자로 정책과 함수를 넘겨준다. 멤버함수도 가능.

```c++
std::future<int> f= std::async(std::launch::async, Sum);
```

정책인자로 async와 deferred가 있다. 전자는 다른 스레드를 만들어서 실행하고, 후자는 호출 스레드에서  
실행하되 처음 요청되는 순간에 실행한다(lazy eval)

```c++
enum class launch : /* unspecified */ {
    async =    /* unspecified */,
    deferred = /* unspecified */,
    /* implementation-defined */
};
(since C++11)

```

어쩄든 호출 후 바로 반환한다. 나중에 필요하면 future.get한다.

```c++
int sum = f.get();
```

get()을 보면 wait()를 호출한다. wait함수는 값이 준비가 되기 까지 블락한다. wait_for라는 함수도  
있다. 이함수는 대신 std::future_status라는 걸 반환한다. 각 반환값은 다음과 같다.

- future_status::deferred : lazy eval을 한다는 의미로 명시적으로 요청될떄만 계산
- future_status::ready : 값 준비
- future_status::timeout : 시간 만료

### std::promise

std::promise는 promise개체를 통해 생성 될 future개체를 통해, 나중에 획득될 값을 저장하는 데 쓰인다.

```c++
void accumulate(std::vector<int>::iterator first,
                std::vector<int>::iterator last,
                std::promise<int> accumulate_promise)
{
    int sum = std::accumulate(first, last, 0);
    accumulate_promise.set_value(sum); // Notify future
}


int main()
{
    // Demonstrate using promise<int> to transmit a result between threads.
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6};
    std::promise<int> accumulate_promise;
    std::future<int> accumulate_future = accumulate_promise.get_future();
    std::thread work_thread(accumulate, numbers.begin(), numbers.end(),
                            std::move(accumulate_promise));

    // future::get() will wait until the future has a valid result and retrieves it.
    // Calling wait() before get() is not needed
    // accumulate_future.wait(); // wait for result
    std::cout << "result=" << accumulate_future.get() << '\n';
    work_thread.join(); // wait for thread completion
```

### std::packaged_task

std::packaged_task()는 함수, 람다식등 호출 가능한 것을 감싸서 비동기적으로 실행되게 한다.
반환값은 future를 통해서 접근 가능하다.

```c++
// unique function to avoid disambiguating the std::pow overload set
int f(int x, int y) { return std::pow(x, y); }

void task_lambda()
{
    std::packaged_task<int(int, int)> task([](int a, int b)
    {
        return std::pow(a, b);
    });
    std::future<int> result = task.get_future();

    task(2, 9);

    std::cout << "task_lambda:\t" << result.get() << '\n';
}

void task_bind()
{
    std::packaged_task<int()> task(std::bind(f, 2, 11));
    std::future<int> result = task.get_future();

    task();

    std::cout << "task_bind:\t" << result.get() << '\n';
}

void task_thread()
{
    std::packaged_task<int(int, int)> task(f);
    std::future<int> result = task.get_future();

    std::thread task_td(std::move(task), 2, 10);
    task_td.join();

    std::cout << "task_thread:\t" << result.get() << '\n';
}

int main()
{
    task_lambda();
    task_bind();
    task_thread();
}
```

#### 참조

- [std::future](https://en.cppreference.com/w/cpp/thread/future)
- [std::launch](https://en.cppreference.com/w/cpp/thread/launch)
- [std::future::wait_for](https://en.cppreference.com/w/cpp/thread/future/wait_for)
- [std::promise](https://en.cppreference.com/w/cpp/thread/promise)
- [std::packaged_task](https://en.cppreference.com/w/cpp/thread/packaged_task)
