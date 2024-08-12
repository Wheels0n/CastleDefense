# Precompiled Headers

## 불필요한 전처리기 작업

프로젝트 속성 - 전처리기에서 Preprocess to a File 속성을 켜고 빌드를하자. 그러면  
obj파일 대신에 i라는 전처리 결과가 나온다.

```i
#line 1614 "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\ucrt\\corecrt_wstdio.h"
    {
        int _Result;
        va_list _ArgList;
        ((void)(__vcrt_assert_va_start_is_not_reference<decltype(_Format)>(), ((void)(__va_start(&_ArgList, _Format)))));
        _Result = _vswprintf_c_l(_Buffer, _BufferCount, _Format, 0, _ArgList);
        ((void)(_ArgList = (va_list)0));
        return _Result;
    }
    #line 1623 "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\ucrt\\corecrt_wstdio.h"


     __declspec(deprecated("This function or variable may be unsafe. Consider using " "_snwprintf_s_l" " instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. " "See online help for details."))
    __inline int __cdecl _snwprintf_l(
            wchar_t*       const _Buffer,
                                                  size_t         const _BufferCount,
                 wchar_t const* const _Format,
                                              _locale_t      const _Locale,
        ...)


#line 1635 "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\ucrt\\corecrt_wstdio.h"
    {
        int _Result;
        va_list _ArgList;
        ((void)(__vcrt_assert_va_start_is_not_reference<decltype(_Locale)>(), ((void)(__va_start(&_ArgList, _Locale)))));

        _Result = _vsnwprintf_l(_Buffer, _BufferCount, _Format, _Locale, _ArgList);

        ((void)(_ArgList = (va_list)0));
        return _Result;
    }
    #line 1646 "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.22621.0\\ucrt\\corecrt_wstdio.h"



    __inline int __cdecl _snwprintf(
            wchar_t*       _Buffer,
                                                  size_t         _BufferCount,
                           wchar_t const* _Format,
        ...)
```

이런게 수 십만 줄이 생성되는 데 이걸 빌드할 떄마다 계속 반복한다. stl처럼 바뀔일도
없는 걸 계속 컴파일하는 건 낭비다.

## Pch

전처리기 - precompiled header 속성을 Yes로 하고 pch로 사용할 헤더를 지정할 수 있다.

- /Yc : pch를 생성한다.
- /Yu : 기존의 pch를 사용한다.

stdafx.cpp만 Yc로 하고 프로젝트 설정은 Yu로 했다.

모든 .cpp 파일에 #include "pch이름.h"을 추가 해줘아한다.

#### 참조

- [Precompiled Headers in C++](https://www.youtube.com/watch?v=eSI4wctZUto)
- [MSDN : PCH](https://learn.microsoft.com/en-us/cpp/build/creating-precompiled-header-files?view=msvc-170)
- [포프 : PCH사용법](https://gamedevforever.com/134)
