cmake_minimum_required(VERSION 3.10)

project(raft)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS raftRPC.proto)

set(NET_SRCS
        ${PROTO_SRCS}
        RaftRPCImp.cpp
        RaftRPCClient.cpp
        Raft.cpp)

add_library(raft SHARED ${NET_SRCS})
