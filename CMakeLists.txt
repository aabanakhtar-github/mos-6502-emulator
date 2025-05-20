cmake_minimum_required(VERSION 3.16)

project(Intel6502Emulator) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED)

set(SOURCES
    src/main.cpp 
    src/mos6502.cpp 
    src/mos6502.h 
    src/types.h 
    src/util.h
)

add_executable(${PROJECT_NAME} ${SOURCES})