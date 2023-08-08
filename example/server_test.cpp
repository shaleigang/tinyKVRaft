//
// Created by slg on 8/8/23.
//

#include "raft/RaftRPCImp.h"
#include <tinyMuduo/net/EventLoop.h>
#include <tinyMuduo/net/InetAddress.h>
#include <tinyMuduo/net/rpc/RpcServer.h>
#include <tinyMuduo/base/Logger.h>

using tmuduo::Logger;

int main() {
    tmuduo::net::EventLoop loop;
    tmuduo::net::InetAddress listenAddr(9981);
    raft::RaftRPCImp imp;
    tmuduo::net::RpcServer server(&loop, listenAddr);
    server.registerService(&imp);
    LOG_INFO("RpcServer UP.");
    server.start();
    loop.loop();
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}