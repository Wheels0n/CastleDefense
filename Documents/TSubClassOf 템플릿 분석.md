## TSubClassOf

TSubclassOf는 특정 클래스의 파생 클래스만 할당하기를 바랄 떄 쓸 수 있다.

```c++
/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	UClass* DamageType;

//vs

/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=Damage)
	TSubclassOf<UDamageType> DamageType;
```

컴파일 후 패널에서 확인 해보면 전자는 아무 UCLass나 선택 가능한 반면 후자는  
UDamageType의 파생클래스만 선택 되게 한다. 이러한 UPROPERTY 안정성에 추가로
C++ 수준의 유형 안정성도 확보 가능하다.

```c++
	UClass* ClassA = UDamageType::StaticClass();

	TSubclassOf<UDamageType> ClassB;

	ClassB = ClassA; // Performs a runtime check

	TSubclassOf<UDamageType_Lava> ClassC;

	ClassB = ClassC; // Performs a compile time check

```

근데 어떻게 마법같이 파생 클래스인지 확인 하는 지 궁금해서 또 SubClassOf.h파일을 열어봤다.

### TIsTSubclassOf

```c++
template <typename T>
struct TIsTSubclassOf
{
	enum { Value = false };
};

template <typename T> struct TIsTSubclassOf<               TSubclassOf<T>> { enum { Value = true }; };
```

기본은 아니라고 판정하는 데 템플릿 특수화로 진짜 하위 클래스이면 참을 반환한다.

### =연산자 오버로딩

```c++
/** Assign from a UClass* (or something implicitly convertible to it). */
template <
	typename U,
	std::enable_if_t<
		!TIsTSubclassOf<std::decay_t<U>>::Value,
		decltype(ImplicitConv<UClass*>(std::declval<U>()))
	>* = nullptr
>
FORCEINLINE TSubclassOf& operator=(U&& From)
{
	Class = From;
	return *this;
}

```

처음에 보고 기겁했다. 하지만 이미 분석을 시작했는데 포기하면 쓰나. 하나씩 파헤쳐 보자.

### std::enable_if

```c++
template< bool B, class T = void >
struct enable_if;


_EXPORT_STD template <bool _Test, class _Ty = void>
struct enable_if {}; // no member "type" when !_Test

template <class _Ty>
struct enable_if<true, _Ty> { // type is _Ty for _Test
    using type = _Ty;
};
```

c++11에서 도입. B가 참이라면 enable_if는 \_Ty 타입을 type이라고 정의한다.
어쩃뜬 정의 한 값이 std::enable_if<평가식, 타입> \* = nullptr 로 구조체
널 포인터가 된다.

### std::deacy_t

```c++
template< class T >
struct decay;
```

함수 인자를 값으로 전달할 때 일어나는 타입 변환을 한다.

- 타입이 만약 U 배열이나 또는 이에 대한 참조라면 멤버 typedef type은 U\*가 된다.
- T가 함수 F 거나 또는 이에 대한 참조라면, 멤버 typeedef type은
  std::add_pointer<F>::type이 된다.
- 그 외 경우 std::remove_cv<std::remove_reference<T>::type>::type 이 된다
- std::remove_cv는 const와 volatale을 제거한다.
- std::remove_reference<T>는 참조를 제거한다.

### declytype()

```c++
decltype(exp)

decltype(a->x) y;       // type of y is double (declared type)
decltype((a->x)) z = y; // type of z is const double& (lvalue expression)
```

decltype은 표현식의 타입을 반환 한다.

### ImplicitConv

```c++
template <typename T>
FORCEINLINE T ImplicitConv(typename TIdentity<T>::Type Obj)
{
	return Obj;
}

```

특정 타입의 개체를 생성하기 위해 암묵적 변환을 사용한다. 명확하게 하고 템플릿에서의  
원치 않는 타입 추론을 피하는 데 유용하다. 다운 캐스팅을 허용하지 않아 C캐스팅과  
static_cast보다 유용하다

### TIdentity<T>

```c++
template <typename T>
struct TIdentity
{
	using Type = T;
	using type = T;
};

template <typename T>
using TIdentity_T = typename TIdentity<T>::Type;

```

그냥 전달 받은 타입과 같을 걸 반환하는 데 함수 인자에서 불필요한 템플릿 인자 추론을
막기 위함이다.

```c++
template <typename T>
void Func1(T Val); // Can be called like Func(123) or Func<int>(123);

template <typename T>
void Func2(typename TIdentity<T>::Type Val); // Must be called like Func<int>(123)

```

### std::declval

```c++
template< class T >
typename std::add_rvalue_reference<T>::type declval() noexcept;
```

현 타입을 참조 타입으로 변환하여 셍상자를 안 거치고도 decltype()의 인자로서
멤버 함수들을 쓸 수 있게 해준다.

##

```c++
/** Assign from another TSubclassOf, only if types are compatible */
template <
	typename OtherT,
	decltype(ImplicitConv<T*>((OtherT*)nullptr))* = nullptr
>
FORCEINLINE TSubclassOf& operator=(const TSubclassOf<OtherT>& Other)
{
	IWYU_MARKUP_IMPLICIT_CAST(OtherT, T);
	Class = Other.Class;
	return *this;
}
```

하나 헷갈렸던 게 어떤 포인터에서 다른 포인터 유형으로 캐스팅되는 건 항상 가능한 걸로
알고있었다. 컴파일은 되고 런타임시에 쓰다가 크래시 나는 줄 알았다.

템플릿은 컴파일 타임에 실제 코드를 생성하기 때문에 변환이 불가능하면 에러가 난다.
이부분은 아리송하다. 컴파일은 잘만된다.

```c++
AIControllerClass = TSubclassOf<USkeletalMesh>();

/** Assign from a UClass*. */
FORCEINLINE TSubclassOf& operator=(UClass* From)
{
	Class = From;
	return *this;
}
```

엉뚱한 클래스를 넣으면 UClass\*로 별다는 검사없이 받는 함수를 부른다.

#### 참조

- [cppreference : std::enable_if](https://en.cppreference.com/w/cpp/types/enable_if)
- [cppreference : std::decay](https://en.cppreference.com/w/cpp/types/decay)
- [cppreference : decltype](https://en.cppreference.com/w/cpp/language/decltype)
- [cppreference : std::declval](https://en.cppreference.com/w/cpp/utility/declval)
