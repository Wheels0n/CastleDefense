# lvalue와 rvalue

c++의 값 범주와 특징에 대해서 알아보자.

## 값 범주

모든 C++ 식에는 형식이 있고 값 범주에 속한다. 값 범주는 평가 중에 임시 개체를 만들고  
복사하고 이동할 때 컴파일러가 따라야하는 규칙의 기초다.

![값 범주](https://learn.microsoft.com/ko-kr/cpp/cpp/media/value_categories.png?view=msvc-170)

- glvalue(“generalized” lvalue) : 평가에서 개체, 비트 필드 또는 함수의 ID를 결정하는 식
- prvalue(“pure” rvalue) : 계산에서 개체 또는 비트 필드를 초기화하거나 표시되는 컨텍스트  
  컨텍스트에 지정된 대로 연산자의 피연산자 값을 계산하는 식
- xvalue(an “eXpiring” value) : 리소스를 재사용할 수 있는 개체 또는 비트 필드를 나타내는  
  glvalue
- lvalue : xvalue가 아닌 glvalue. 프로그램에서 접근 가능한 주소가 있다. 예를 들면 변수, 배열  
  요소, lvalue참조, 비트필드, 공용 구조체 및 클래스 멤버를 반환하는 함수 호출, const변수, 문자열
  리터럴 등이 있다.
- rvalue : prvalue 또는 xvalue. 프로그램에서 접근 가능한 주소가 없다. 예를 들면 리터럴, 비참조  
  형식을 반환하는 함수 호출 및 식 평가 중에 생성되어 컴파일러만 접근할 수 있는 임시 개체가 있다.

## rvalue 참조와 std::move()

과거에 값으로 개체를 반환하는 함수를 호출해서 그 결과를 다른 개체에 넣으면 복사가 두 번이나 발생했다.
이를 ravlue 참조와 move연산자로 극복했다. &&을 rvalue 앞에 붙여주면 rvalue참조가 된다.

std:move(val)를 호출하면 rvalue참조를 반환 하거나 lvalue를 rvalue로 변환한다.

## 이동 생성자와 이동 대입자

ravlue 참조를 받는 이동생성자와 이동대입자는 다른 개체 멤버 변수들의 소유권을 가져온다. 복사 생성자와  
달리 메모리 재할당을 하지 않는다.

## std::forawrd

템플릿을 쓰다보면 이 함수가 등장한다. lvalue 참조면 lvalue참조로 ravlue참조면 rvalue참조를 반환한다.  
당연한 걸 왜 굳이 이렇게 함수로 뻈는가 싶었다. 문제는 함수 매개변수처럼 이름이 있는 값들은 항상 lvaue로  
평가한다는 것이다. rvalue참조로 선언되어 있어도말이다. 그래서 다른 함수에 인자를 전달하는 템플릿 함수의  
경우 move를 보존하기가 어려워 진다.

```c++
// forward example
#include <utility>      // std::forward
#include <iostream>     // std::cout

// function with lvalue and rvalue reference overloads:
void overloaded (const int& x) {std::cout << "[lvalue]";}
void overloaded (int&& x) {std::cout << "[rvalue]";}

// function template taking rvalue reference to deduced type:
template <class T> void fn (T&& x) {
  overloaded (x);                   // always an lvalue
  overloaded (std::forward<T>(x));  // rvalue if argument is rvalue
}

int main () {
  int a;

  std::cout << "calling fn with lvalue: ";
  fn (a);
  std::cout << '\n';

  std::cout << "calling fn with rvalue: ";
  fn (0);
  std::cout << '\n';

  return 0;
}
/* 결과
calling fn with lvalue: [lvalue][lvalue]
calling fn with rvalue: [lvalue][rvalue]
/
```

#### 참조

- [MSDN: Lvalue 및 Rvalue](https://learn.microsoft.com/ko-kr/cpp/cpp/lvalues-and-rvalues-visual-cpp?view=msvc-170)
- [Value categories](https://en.cppreference.com/w/cpp/language/value_category)
- [Move constructors](https://en.cppreference.com/w/cpp/language/move_constructor)
- [Move assignment operator](https://en.cppreference.com/w/cpp/language/move_assignment)
- [씹어먹는 C++ - <12 - 1. 우측값 레퍼런스와 이동 생성자>](https://modoocode.com/227)
- [std::forward](https://cplusplus.com/reference/utility/forward/)
- [Universal References in C++11](https://isocpp.org/blog/2012/11/universal-references-in-c11-scott-meyers)
