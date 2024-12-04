cmake_minimum_required(VERSION 3.27)
project(trainGramm)

set(CMAKE_CXX_STANDARD 20)
if(WIN32)
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()
add_executable(trainGramm main.cpp
        Converter/GrammaToNFA.h)


