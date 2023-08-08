//
// Created by slg on 8/8/23.
//

#include "raft/RaftRPCClient.h"

#include <tinyMuduo/base/Logger.h>
#include <tinyMuduo/net/EventLoop.h>
#include <tinyMuduo/net/InetAddress.h>

#include <unistd.h>

using namespace tmuduo;
using namespace tmuduo::net;

void add(raft::RaftRPCClient* client) {
    client->runEvery(1);
    while(1) {
        sleep(2);
        LOG_DEBUG("add task");
        client->makeVoteRequest();
        client->makeAppendRequest();
    }
}

int main() {
    EventLoop loop;
    InetAddress serverAddr("127.0.0.1", 9981);

    raft::RaftRPCClient rpcClient(&loop, serverAddr);
    rpcClient.connect();
    std::thread t(add, &rpcClient);
    loop.loop();

    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}