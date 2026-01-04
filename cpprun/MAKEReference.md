# cpprun/CMakeLists.txt 설명 + 특정 .cpp마다 실행 파일을 따로 만드는 방법

현재 CMakeLists.txt는 **C++20 설정 + Boost(헤더) 탐색 + `src/main.cpp`로 단일 실행 파일(cpprun) 생성**을 합니다. 여기에 `add_executable()`를 추가하면 **원하는 `특정파일명.cpp`마다 별도 실행 파일(타겟)** 을 쉽게 만들 수 있고, 빌드도 `cmake --build ... --target <타겟명>`으로 골라서 할 수 있습니다.

## 예시 파일
[file:///Users/donghun2/workspace/Security/tls-study/cpprun/CMakeLists.txt](file:///Users/donghun2/workspace/Security/tls-study/cpprun/CMakeLists.txt)

### 1) 현재 CMakeLists.txt가 하는 일(줄 단위로)

- `cmake_minimum_required(VERSION 3.20)`  
  - 최소 CMake 버전 요구(3.20 이상).
- `project(cpprun LANGUAGES CXX)`  
  - 프로젝트 이름 `cpprun`, C++ 사용.
- `set(CMAKE_CXX_STANDARD 20)` / `set(CMAKE_CXX_STANDARD_REQUIRED ON)`  
  - 컴파일 표준을 **C++20**으로 강제.
- `find_package(Boost REQUIRED)`  
  - Boost를 찾고, 없으면 **설정 단계에서 실패**(지금 겪은 “Could NOT find Boost”가 여기서 발생).
- `add_executable(cpprun src/main.cpp)`  
  - 실행 파일 타겟 `cpprun`을 만들고, 소스는 `src/main.cpp` 하나를 사용.
- `target_include_directories(cpprun PRIVATE ...)`  
  - 컴파일 시 include 경로 추가:
    - `${Boost_INCLUDE_DIRS}`: Boost 헤더 경로
    - `${CMAKE_CURRENT_SOURCE_DIR}/include`: 프로젝트의 `include/` 폴더

즉, 구조는 “단일 실행 파일(연습용)”에 딱 맞춘 최소 형태입니다.

---

### 2) `main.cpp` 말고 “특정 파일명.cpp”마다 실행 파일을 따로 만드는 방법
CMake에서는 “실행 파일 = 타겟”이라서, **`add_executable()`를 여러 번** 호출하면 됩니다.

예를 들어 `src/hello.cpp`, `src/psk_demo.cpp` 같은 파일을 각각 독립 실행으로 만들고 싶다면:

````cmake
// filepath: /Users/donghun2/workspace/Security/tls-study/cpprun/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(cpprun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

# 공통 include 경로를 함수로 묶어 중복 제거
function(add_cpprun_exe target source_file)
  add_executable(${target} ${source_file})
  target_include_directories(${target} PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  )
endfunction()

# 기존 main
add_cpprun_exe(cpprun src/main.cpp)

# 새로 추가: 원하는 파일마다 실행 파일 생성
add_cpprun_exe(hello src/hello.cpp)
add_cpprun_exe(psk_demo src/psk_demo.cpp)