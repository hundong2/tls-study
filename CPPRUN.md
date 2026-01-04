# CPPRUN
C++20 + Boost를 “학습용으로 빠르게 파일 추가/빌드/실행”하기 위한 최소 템플릿입니다.

이 템플릿은 `tls-study/cpprun/` 아래에 있으며, 두 가지 빌드 방식을 모두 지원합니다.
- `make` (내부적으로 CMake를 호출하는 래퍼)
- `cmake --build`

## 예시 파일
- https://github.com/ttroy50/cmake-examples

---

## 0) 디렉토리
- `cpprun/CMakeLists.txt`: CMake 설정(C++20, Boost 헤더)
- `cpprun/Makefile`: `make`로도 빌드되게 해주는 래퍼
- `cpprun/src/main.cpp`: C++20(`std::span`) + Boost 사용 예시
- `cpprun/include/cpprun/hello.hpp`: 헤더 추가 예시

---

## 1) Ubuntu 컨테이너(ubuntu-dev2)에서 1회 설치
`ubuntu:latest`는 기본적으로 컴파일러/빌드툴/Boost 헤더가 없을 수 있습니다.
컨테이너 안에서 아래를 1회 실행하세요.

```sh
apt update
apt install -y build-essential cmake libboost-dev
```

확인:
```sh
g++ --version
cmake --version
```

---

## 2) 빌드/실행 방법 (docker + ubuntu-dev2)
아래 명령들은 macOS 호스트에서 실행합니다(컨테이너 안으로 들어가지 않고도 실행 가능).

> 중요: CMake는 `build` 디렉토리에 절대경로 캐시(CMakeCache.txt)를 남깁니다.
> 그래서 호스트(macOS)에서 만든 build를 컨테이너(/workspace)에서 재사용하면 경로 불일치 에러가 납니다.
> 이를 피하려고 `cpprun/Makefile`은 기본 build 디렉토리를 `build-OS-ARCH` 형태로 분리합니다.

### A) make로 빌드(권장: 가장 짧음)
```sh
./dev-ubuntu-dev2.sh make -C cpprun
```

실행까지:
```sh
./dev-ubuntu-dev2.sh make -C cpprun run
```

### B) 순수 CMake로 빌드
```sh
./dev-ubuntu-dev2.sh bash -lc "cd cpprun && cmake -S . -B build-Linux-$(uname -m)"
./dev-ubuntu-dev2.sh bash -lc "cd cpprun && cmake --build build-Linux-$(uname -m) -j"
```

실행:
```sh
./dev-ubuntu-dev2.sh bash -lc "./cpprun/build-Linux-$(uname -m)/cpprun"
```

호스트(macOS)에서 빌드할 땐 예를 들어:
```sh
cd cpprun
cmake -S . -B build-Darwin-$(uname -m)
cmake --build build-Darwin-$(uname -m) -j
./build-Darwin-$(uname -m)/cpprun
```

---

## 3) 새 .cpp/.h/.hpp 파일 추가(학습용 워크플로우)
### 3-1) 새 파일 만들기
예: `cpprun/src/day01.cpp`, `cpprun/include/cpprun/day01.hpp`

### 3-2) CMakeLists.txt에 소스 파일을 추가
현재는 최소 구성을 위해 `src/main.cpp`만 명시되어 있습니다.
새 cpp를 빌드에 포함하려면 `cpprun/CMakeLists.txt`의 `add_executable(cpprun ...)`에 추가하세요.

예:
```cmake
add_executable(cpprun
  src/main.cpp
  src/day01.cpp
)
```

### 3-3) 다시 빌드
```sh
./dev-ubuntu-dev2.sh make -C cpprun
```

---

## 4) 자주 나는 문제
### 4-0) CMakeCache 경로 불일치 에러
에러 예:
`CMakeCache.txt directory ... is different than the directory ... where CMakeCache.txt was created`

해결(빌드 디렉토리 삭제 후 재시도):
```sh
rm -rf cpprun/build-*
```

### 4-1) `cmake: command not found`
컨테이너 안에 CMake가 없습니다.

```sh
apt update
apt install -y cmake
```

### 4-2) `fatal error: boost/...: No such file or directory`
Boost 헤더가 없습니다.

```sh
apt update
apt install -y libboost-dev
```

### 4-3) C++20 옵션 확인
```sh
./dev-ubuntu-dev2.sh bash -lc "g++ -std=c++20 -dM -E -x c++ /dev/null | grep __cplusplus"
```

---

## 추가 자료
- https://cmake.org/cmake/help/latest/guide/tutorial/index.html
- https://www.boost.org/doc/libs/release/more/getting_started/index.html
- https://cli.github.com/
