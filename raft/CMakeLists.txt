find_package(Protobuf REQUIRED)

#aux_source_directory(. NET_SRCS)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS raftRPC.proto)

set(NET_SRCS
        ${PROTO_SRCS}
        RaftRPCImp.cpp
        RaftRPCClient.cpp
        Raft.cpp)

add_library(raft SHARED ${NET_SRCS})

