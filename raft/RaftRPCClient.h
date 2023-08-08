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
typedef std::shared_ptr<RequestAppendArgs> RequestAppendArgsPtr;

class RaftRPCClient {
public:
    typedef std::pair<uint32_t, RequestAppendArgsPtr> AppendReplyCallbackMsg;
    typedef std::function<void(RequestVoteReply*)> VoteReplyCallback;
    typedef std::function<RequestAppendArgsPtr (AppendReplyCallbackMsg, RequestAppendReply*)> AppendReplyCallback;

    RaftRPCClient(EventLoop* loop, const InetAddress& serverAddr);

    void makeVoteRequest(RequestVoteArgsPtr& request);
    void makeAppendRequest(uint32_t target_server, RequestAppendArgsPtr& request);

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    void setVoteReplyCallback(const VoteReplyCallback& cb) { voteReplyCallback_ = cb; }
    void setAppendReplyCallback(const AppendReplyCallback& cb) { appendReplyCallback_ = cb; }

private:
    void voteRequest(RequestVoteArgsPtr& request);
    void appendRequest(uint32_t target_server, RequestAppendArgsPtr& request);

    void onVoteResponse(RequestVoteReply* response);
    void onAppendResponse(AppendReplyCallbackMsg msg, RequestAppendReply* response);

    void onConnection(const TcpConnectionPtr& conn);

private:
    EventLoop* loop_;
    RpcClient client_;
    RaftRPC::Stub* stub_;

    VoteReplyCallback voteReplyCallback_;
    AppendReplyCallback appendReplyCallback_;
};

typedef std::shared_ptr<RaftRPCClient> RaftRPCClientPtr;

}



#endif //TINYKVRAFT_RAFTRPCCLIENT_H
