cmake_minimum_required(VERSION 3.16)

project(Intel6502Emulator) 
set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

include(FetchContent)

FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3  # Use latest stable
)

FetchContent_MakeAvailable(json)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.3 # or latest stable
)

FetchContent_MakeAvailable(Catch2)

set(SOURCES
    src/mos6502.cpp 
    src/mos6502.h
    src/components.cpp 
    src/components.h 
    src/types.h 
    src/util.h
)


add_library(EmulatorCore ${SOURCES})
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} EmulatorCore)

set(TESTS
    testing/harte_test.h
    testing/lda.cpp
    testing/harte_test.cpp
)

add_executable(tests ${TESTS} )
target_link_libraries(tests Catch2::Catch2WithMain nlohmann_json::nlohmann_json EmulatorCore)
target_include_directories(tests PRIVATE src/)

include(CTest)
include(Catch)
catch_discover_tests(tests)