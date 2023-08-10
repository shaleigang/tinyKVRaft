cmake_minimum_required(VERSION 3.10)

project(raft)
set(CMAKE_CXX_STANDARD 17)

set(NET_SRCS
        kvstore.cpp)

add_library(kv SHARED ${NET_SRCS})