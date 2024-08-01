# new와 delete연산자

메모리풀과 오브젝트풀을 공부하기 전에 new/delete연산자를 살펴보자

## new/delete 오버로딩

new 연산자가 포함된 표현식은 new연산자를 지정된 타입의 바이트 크기를 인자로 받아 실행한다.  
성공하면 필요시에 자동으로 초기화 하거나 또는 생성자가 불린다. 최종 결과는 포인터다.

```c++
std::cout << "1: ";
  MyClass * p1 = new MyClass;
      // allocates memory by calling: operator new (sizeof(MyClass))
      // and then constructs an object at the newly allocated space

```

new와 delete 연산자도 오버로딩이 된다!. 연산자 오버로딩이 되니까 어쨰보면 당연하다.
방식은 두 개가 있다.

### 전역 교체

어느 소스파일 이건 프로그램내에 선언된 유저가 만든 비멤버 함수는 기본 버전을 대체한다.  
다음과 같은 경우 정상적으로 작동하지 않는다.

- 두 개이상의 오버로드 버전이 존재
- 인라인으로 선언됨
- 전역 네임스페이스가 아님
- 전역 static 비멤버 함수로 정의됨

```c++

// no inline, required by [replacement.functions]/3
void* operator new(std::size_t sz)
{
    std::printf("1) new(size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void *ptr = std::malloc(sz))
        return ptr;

    throw std::bad_alloc{}; // required by [new.delete.single]/3
}

// no inline, required by [replacement.functions]/3
void* operator new[](std::size_t sz)
{
    std::printf("2) new[](size_t), size = %zu\n", sz);
    if (sz == 0)
        ++sz; // avoid std::malloc(0) which may return nullptr on success

    if (void *ptr = std::malloc(sz))
        return ptr;

    throw std::bad_alloc{}; // required by [new.delete.single]/3
}

void operator delete(void* ptr) noexcept
{
    std::puts("3) delete(void*)");
    std::free(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept
{
    std::printf("4) delete(void*, size_t), size = %zu\n", size);
    std::free(ptr);
}

void operator delete[](void* ptr) noexcept
{
    std::puts("5) delete[](void* ptr)");
    std::free(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept
{
    std::printf("6) delete[](void*, size_t), size = %zu\n", size);
    std::free(ptr);
}

int main()
{
    int* p1 = new int;
    delete p1;

    int* p2 = new int[10]; // guaranteed to call the replacement in C++11
    delete[] p2;
}
//결과
// Compiled with GCC-5 in C++17 mode to obtain the following:
//1) op new(size_t), size = 4
//4) op delete(void*, size_t), size = 4
//2) op new[](size_t), size = 40
//5) op delete[](void* ptr)
```

다만 죄다 대체 해버리니 문제가 될 수 있다.

### 특정 클래스 오버로드

어떤 클래스의 public static함수로 정의 해도 된다. static은 붙이던 안 붙이던 전역  
멤버함수이다. 이 오버로드는 오로지 그 클래스의 개체 할당에만 적용되게 된다.

new 표현식은 클래스 범위->전역범위 순으로 적절한 함수를 찾아본다. 클래스 범위 오버로드가  
전역범위 오버로드를 가린다.

```c++
struct X
{
    X() { throw std::runtime_error(""); }

    // custom placement new
    static void* operator new(std::size_t count, bool b)
    {
        std::cout << "custom placement new called, b = " << b << '\n';
        return ::operator new(count);
    }

    // custom placement delete
    static void operator delete(void* ptr, bool b)
    {
        std::cout << "custom placement delete called, b = " << b << '\n';
        ::operator delete(ptr);
    }
};

int main()
{
    try
    {
        [[maybe_unused]] X* p1 = new (true) X;
    }
    catch (const std::exception&)
    {}
}
/* 결과
custom placement new called, b = 1
custom placement delete called, b = 1
/
```

이것도 문제가 있는 게 모든 클래스에다가 이짓을 할수는 없는 노릇이다.

## placement new

이미 할당된 메모리에다가 생성자를 호출하는 방법이 있다! 포인터를 new연산자에
넘겨주면 직접 할당하지 않고 포인터가 가리키는 곳에 적절한 초기화 작업을 수행한다.

```c++
std::cout << "3: ";
  new (p2) MyClass;
```

malloc/free를 이용해서 메모리만 할당하는 allocator클래스를 생성하였다.

```c++
void* BaseAllocator::Alloc(int size)
{
    return malloc(size);
}

void BaseAllocator::Release(void* ptr)
{
    free(ptr);
}

```

static 멤버함수인 할당자로 타입크기만큼 할당하고 new연산자로 생성자를 부른다.  
delete의 경우 바로 소멸자를 부르고 메모리를 수거한다.

```c++
template<typename T, typename... Args>
T* xnew(Args&&... args)
{
	T* memory = static_cast<T*>(BaseAllocator::Alloc(sizeof(T));
	new(memory)T(std::forward<Args>(args)...);
	return memory;
}

template<typename T>
void xdelete(T* pObj)
{
	pObj->~T();
	BaseAllocator::Release(pObj);
}
```

#### 참조

- [operator new, operator new[]](https://en.cppreference.com/w/cpp/memory/new/operator_new)
- [operator new](https://cplusplus.com/reference/new/operator%20new/)
- [std::forward](https://cplusplus.com/reference/utility/forward/)
