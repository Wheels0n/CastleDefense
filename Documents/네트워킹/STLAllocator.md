# STLAllocator

STL컨테이너를 이용할일이 많은데 따로 지정하지 않으면 내부적으로
new/delete를 한다. 이를 고쳐 보자.

## STL 컨테이너 할당 방식

std::vector를 보면 템플릿 기본인자로 std::allocator 클래스를 받는다. 이 클래스는
할당/해제시에 그냥 new/delete한다.

```c++
_EXPORT_STD template <class _Ty, class _Alloc = allocator<_Ty>>
class vector { // varying size array of values
```

물론 그렇다고 아무 할당자 클래스를 집어넣는다고 저절로 되는 건아니다. c++11에서는 allocator_traits  
라는 걸 제공하는 데 이 템플릿은 할당자 타입에 대한 범용적인 인터페이스를 제공한다. 이 인터페이스를
통해 value_type과 allocate/deallocate를 제공하는 어느 클래스든 할당자로 쓸 수 있다.

```c++

template<typename T>
class StlAllocator
{
public:
	typedef T value_type;

	StlAllocator() {}

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) { }

	T* allocate(size_t n)
	{
		const int size = static_cast<int>(n * sizeof(T));
		return static_cast<T*>(StompAllocator::Alloc(size));
	}

	void deallocate(T* ptr, size_t n)
	{
		StompAllocator::Release(ptr);
	}
};
```

#### 참조

- [std::allocator<T>::allocate](https://en.cppreference.com/w/cpp/memory/allocator/allocate)
- [Implementing a custom memory allocator](https://docs.ros.org/en/foxy/Tutorials/Advanced/Allocator-Template-Tutorial.html)
- [std::allocator_traits](https://cplusplus.com/reference/memory/allocator_traits/#google_vignette)
