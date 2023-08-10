cmake_minimum_required(VERSION 3.10)

project(tinyKVRaft)
set(CMAKE_CXX_STANDARD 17)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(raft)
add_subdirectory(kv)

add_executable(server server.cpp)
link_libraries(server kv raft)
