cmake_minimum_required(VERSION 3.29)
project(parserek)

set(CMAKE_CXX_STANDARD 20)

add_executable(parserek
        tsCommon.h
        tsTransportStream.cpp
        tsTransportStream.h
        TS_parser.cpp)
