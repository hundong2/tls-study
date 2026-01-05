# CMakeLists.txt(현재 파일) 상세 설명

이 CMakeLists.txt는 **C++20 프로젝트(cpprun)** 를 빌드하면서, 별도 소스인 **`mpz.cc/mpz.h`(GMP 의존)** 를 정적 라이브러리로 만들고(`cpprun_mpz`), 메인 실행파일(`cpprun`)과 테스트(`cpprun_tests`, GoogleTest)를 **같이** 빌드/실행할 수 있게 구성한 파일입니다.

## 예시 파일
[CMake 공식 튜토리얼 Step1 CMakeLists.txt (raw)](https://raw.githubusercontent.com/Kitware/CMake/master/Help/guide/tutorial/Step1/CMakeLists.txt)

## 답변

아래는 위 파일을 “위에서 아래로” 초보자 기준으로 해석한 내용입니다.

---

## 1) 프로젝트 기본 설정

```cmake
cmake_minimum_required(VERSION 3.20)
project(cpprun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

- `cmake_minimum_required(VERSION 3.20)`  
  이 프로젝트는 **최소 CMake 3.20**이 필요하다는 뜻입니다.
- `project(cpprun LANGUAGES CXX)`  
  프로젝트 이름은 `cpprun`, 언어는 **C++**를 사용합니다.
- `CMAKE_CXX_STANDARD 20`  
  C++20 모드로 컴파일합니다. (`std::span` 같은 C++20 기능 사용 가능)
- `CMAKE_CXX_STANDARD_REQUIRED ON`  
  컴파일러가 C++20을 지원하지 않으면 “대충 낮춰서 빌드”하지 말고 **에러를 내라**는 의미입니다.

---

## 2) macOS에서 SDK 경로 문제 예방 (표준 헤더 못 찾는 문제)

```cmake
if(APPLE AND (NOT CMAKE_OSX_SYSROOT))
  execute_process(
    COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE _sdk_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  if(_sdk_path)
    set(CMAKE_OSX_SYSROOT "${_sdk_path}" CACHE PATH "macOS SDK" FORCE)
  endif()
endif()
```

- macOS에서 가끔 `<iostream>` 같은 **표준 헤더를 못 찾는 환경**이 생깁니다(특히 Command Line Tools/SDK 설정이 어긋난 경우).
- 이 블록은 `xcrun --show-sdk-path`로 **현재 SDK 경로를 자동 감지**해서 `CMAKE_OSX_SYSROOT`에 강제로 넣어 줍니다.
- 결과적으로 “헤더를 못 찾는다” 류의 환경 문제를 줄이는 목적입니다.

---

## 3) Boost 찾기

```cmake
find_package(Boost REQUIRED)
```

- Boost 라이브러리를 찾습니다.
- `REQUIRED`라서 Boost를 못 찾으면 **CMake configure 단계에서 실패**합니다.
- 이 파일에서는 `main.cpp`에서 Boost를 include 하므로 필요합니다.

---

## 4) GMP(gmp / gmpxx) 찾기: mpz.cc의 의존성

`mpz.h`는 `#include <gmpxx.h>`를 사용합니다. 즉, **GMP의 C++ 바인딩(gmpxx)** 이 필요합니다.

### 4-1) pkg-config로 먼저 시도

```cmake
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(GMP QUIET IMPORTED_TARGET gmp)
  pkg_check_modules(GMPXX QUIET IMPORTED_TARGET gmpxx)
endif()
```

- `pkg-config`가 있으면, 시스템에 설치된 `gmp`, `gmpxx` 정보를 가져와서
  - `PkgConfig::GMP`
  - `PkgConfig::GMPXX`
  같은 **imported target**을 만들어줍니다.
- `QUIET`이라서 없으면 조용히 넘어가고, 이후 다른 방법으로 탐색합니다.

---

## 5) mpz 코드를 “정적 라이브러리”로 분리해서 빌드

```cmake
add_library(cpprun_mpz STATIC
  tls/lib/mpz.cc
)
target_include_directories(cpprun_mpz PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/tls/lib
)
```

- `cpprun_mpz`라는 **정적 라이브러리(.a)** 를 만듭니다.
- 소스는 `tls/lib/mpz.cc` 입니다.
- `target_include_directories(... PUBLIC ...)` 의미:
  - `cpprun_mpz`를 컴파일할 때도 이 include 경로를 쓰고
  - **cpprun_mpz를 링크하는 다른 타겟도** 같은 include 경로를 “상속” 받습니다.
- 그래서 `main.cpp`에서 `#include "mpz.h"`가 가능해집니다.  
  (상대경로로 `../...` 적는 대신 “컴파일러가 찾는 include 경로”에 `tls/lib`를 등록한 것)

---

## 6) cpprun_mpz에 GMP 링크하기 (여러 경로로 시도)

### 6-1) pkg-config로 찾았으면 그 타겟을 링크

```cmake
if(TARGET PkgConfig::GMPXX)
  target_link_libraries(cpprun_mpz PUBLIC PkgConfig::GMPXX)
endif()
if(TARGET PkgConfig::GMP)
  target_link_libraries(cpprun_mpz PUBLIC PkgConfig::GMP)
endif()
```

- gmpxx, gmp를 각각 링크합니다.
- `PUBLIC`이라, `cpprun_mpz`를 쓰는 최종 실행파일도 필요한 링크 설정을 같이 물려받을 수 있습니다(환경에 따라 유용).

### 6-2) pkg-config가 실패하면 find_library로 직접 탐색

```cmake
if(NOT TARGET PkgConfig::GMPXX)
  find_library(GMPXX_LIBRARY NAMES gmpxx)
  if(GMPXX_LIBRARY)
    target_link_libraries(cpprun_mpz PUBLIC ${GMPXX_LIBRARY})
  endif()
endif()
...
```

- `libgmpxx`를 직접 찾아 링크합니다.
- gmp도 동일하게 처리합니다.

### 6-3) 최종적으로 못 찾으면 빌드 중단(FATAL_ERROR)

```cmake
if((NOT TARGET PkgConfig::GMPXX) AND (NOT GMPXX_LIBRARY))
  message(FATAL_ERROR "gmpxx not found. Install GMP ...")
endif()
...
```

- `gmpxx` 또는 `gmp`를 못 찾으면 **설치하라고 안내하고 configure를 실패**시킵니다.
- macOS에서는 보통:
  - `brew install gmp`

---

## 7) 메인 실행파일(cpprun) 빌드

```cmake
add_executable(cpprun
  src/main.cpp
)
```

- `src/main.cpp`로 실행파일 `cpprun`을 만듭니다.

```cmake
target_include_directories(cpprun PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
```

- `cpprun`이 컴파일할 때만 필요한 include 경로들입니다(`PRIVATE`).
- Boost 헤더 경로, 프로젝트의 `include` 폴더 등을 추가합니다.

```cmake
target_link_libraries(cpprun PRIVATE cpprun_mpz)
```

- 메인 실행파일이 `cpprun_mpz`를 링크합니다.
- 이 덕분에 `main.cpp`에서 `mpz` 관련 함수를 호출할 수 있고,
- 위에서 `cpprun_mpz`에 `PUBLIC`으로 걸어둔 include/link 설정도 필요한 만큼 전달됩니다.

---

## 8) 테스트(CTest + GoogleTest) 구성: BUILD_TESTING 켜졌을 때만

```cmake
include(CTest)
if(BUILD_TESTING)
  ...
endif()
```

- `include(CTest)`는 CTest(테스트 러너) 사용 준비를 합니다.
- `BUILD_TESTING`은 보통 `cmake -DBUILD_TESTING=ON ..`으로 켭니다.

### 8-1) GoogleTest를 FetchContent로 다운로드/빌드

```cmake
include(FetchContent)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)
```

- 시스템에 gtest가 없어도 CMake가 zip을 받아서 빌드하게 합니다.
- `INSTALL_GTEST OFF`는 “설치 단계는 하지 말자”는 옵션(프로젝트 내부에서만 쓰려는 목적).

### 8-2) 테스트 실행파일 만들고 링크

```cmake
add_executable(cpprun_tests
  tests/mpz_test.cc
)
target_link_libraries(cpprun_tests PRIVATE
  GTest::gtest_main
  cpprun_mpz
)
```

- `tests/mpz_test.cc`를 빌드해서 `cpprun_tests`라는 테스트 바이너리를 만듭니다.
- `GTest::gtest_main`(gtest 메인 제공) + `cpprun_mpz`(테스트 대상 코드) 링크.

### 8-3) ctest에서 자동으로 테스트 검색/등록

```cmake
include(GoogleTest)
gtest_discover_tests(cpprun_tests)
```

- `ctest`를 실행하면 gtest 테스트 케이스들을 자동으로 찾아서 등록합니다.
- 그래서 별도 `add_test(...)`를 일일이 안 써도 됩니다.

---

## 자주 헷갈리는 포인트(초보자용 요약)

- **헤더 include는 소스에서 상대경로로 억지로 맞추는 것보다**,  
  `target_include_directories()`로 “찾을 경로”를 CMake에 등록하는 게 정석.
- GMP는 **헤더(gmpxx.h)** 뿐 아니라 **링커 단계에서 gmpxx/gmp 라이브러리 링크**가 필요.
- 테스트는 `BUILD_TESTING=ON`일 때만 활성화되고, `ctest`로 실행 가능.

---

### 추가 자료
- [CMake: target_include_directories 공식 문서](https://cmake.org/cmake/help/latest/command/target_include_directories.html)
- [GoogleTest CMake Quickstart](https://google.github.io/googletest/quickstart-cmake.html)
- [CTest 소개](https://cmake.org/cmake/help/latest/manual/ctest.1.html)