cmake_minimum_required(VERSION 3.20)
project(cpprun LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Some environments (especially fresh macOS setups) can mis-detect the SDK path.
# If standard headers like <iostream> are not found, this often fixes it.
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

find_package(Boost REQUIRED)

# mpz.cc uses GMP C++ bindings (gmpxx)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(GMP QUIET IMPORTED_TARGET gmp)
  pkg_check_modules(GMPXX QUIET IMPORTED_TARGET gmpxx)
endif()

add_library(cpprun_mpz STATIC
  tls/lib/mpz.cc
)
target_include_directories(cpprun_mpz PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/tls/lib
)
if(TARGET PkgConfig::GMPXX)
  target_link_libraries(cpprun_mpz PUBLIC PkgConfig::GMPXX)
endif()
if(TARGET PkgConfig::GMP)
  target_link_libraries(cpprun_mpz PUBLIC PkgConfig::GMP)
endif()
if(NOT TARGET PkgConfig::GMPXX)
  find_library(GMPXX_LIBRARY NAMES gmpxx)
  if(GMPXX_LIBRARY)
    target_link_libraries(cpprun_mpz PUBLIC ${GMPXX_LIBRARY})
  endif()
endif()
if(NOT TARGET PkgConfig::GMP)
  find_library(GMP_LIBRARY NAMES gmp)
  if(GMP_LIBRARY)
    target_link_libraries(cpprun_mpz PUBLIC ${GMP_LIBRARY})
  endif()
endif()
if((NOT TARGET PkgConfig::GMPXX) AND (NOT GMPXX_LIBRARY))
  message(FATAL_ERROR "gmpxx not found. Install GMP (e.g. macOS: 'brew install gmp', Ubuntu: 'sudo apt-get install -y libgmp-dev').")
endif()
if((NOT TARGET PkgConfig::GMP) AND (NOT GMP_LIBRARY))
  message(FATAL_ERROR "gmp not found. Install GMP (e.g. macOS: 'brew install gmp', Ubuntu: 'sudo apt-get install -y libgmp-dev').")
endif()

add_executable(cpprun
  src/main.cpp
)

target_include_directories(cpprun PRIVATE
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cpprun PRIVATE cpprun_mpz)

include(CTest)
if(BUILD_TESTING)
  include(FetchContent)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  )
  FetchContent_MakeAvailable(googletest)

  add_executable(cpprun_tests
    tests/mpz_test.cc
  )
  target_link_libraries(cpprun_tests PRIVATE
    GTest::gtest_main
    cpprun_mpz
  )

  include(GoogleTest)
  gtest_discover_tests(cpprun_tests)
endif()
