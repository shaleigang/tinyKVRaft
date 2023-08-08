//
// Created by slg on 8/7/23.
//

#ifndef TINYKVRAFT_RAFTRPCCLIENT_H
#define TINYKVRAFT_RAFTRPCCLIENT_H

#include "raftRPC.pb.h"
#include <tinyMuduo/base/Logger.h>
#include <tinyMuduo/net/EventLoop.h>
#include <tinyMuduo/net/InetAddress.h>
#include <tinyMuduo/net/rpc/RpcChannel.h>
#include <tinyMuduo/net/rpc/RpcClient.h>
#include <tinyMuduo/net/TcpConnection.h>

#include <memory>

namespace raft {

class Raft;

using tmuduo::net::EventLoop;
using tmuduo::net::InetAddress;
using tmuduo::net::TcpConnectionPtr;
using tmuduo::Logger;
using tmuduo::net::RpcClient;

typedef std::shared_ptr<RequestVoteArgs> RequestVoteArgsPtr;
typedef std::shared_ptr<RequestVoteReply> RequestVoteReplyPtr;
typedef std::shared_ptr<RequestAppendArgs> RequestAppendArgsPtr;
typedef std::shared_ptr<RequestAppendReply> RequestAppendReplyPtr;

class RaftRPCClient {
public:

    RaftRPCClient(EventLoop* loop, const InetAddress& serverAddr);

    // TODO: FIX THIS
    void makeVoteRequest();
    void makeAppendRequest();

    void runEvery(double second) {
        loop_->runEvery(1, std::bind(&RaftRPCClient::runFunc, this));
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

private:
    void voteRequest();
    void appendRequest();

    void onVoteResponse(RequestVoteReply* response);
    void onAppendResponse(RequestAppendReply* response);

    void onConnection(const TcpConnectionPtr& conn);

    void runFunc() {
        LOG_INFO("cycle run...");
    }

private:
    EventLoop* loop_;
    RpcClient client_;
    RaftRPC::Stub* stub_;
    Raft* raft_;
};

typedef std::shared_ptr<RaftRPCClient> RaftRPCClientPtr;

}



#endif //TINYKVRAFT_RAFTRPCCLIENT_H
