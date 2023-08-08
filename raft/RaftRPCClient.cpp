//
// Created by slg on 8/7/23.
//

#include "RaftRPCClient.h"

#include <tinyMuduo/net/Callback.h>

using namespace raft;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using tmuduo::net::RpcChannel;

RaftRPCClient::RaftRPCClient(tmuduo::net::EventLoop *loop, const tmuduo::net::InetAddress &serverAddr)
  : loop_(loop),
    client_(loop, serverAddr) {
    client_.setNewConnectionCallback(std::bind(&RaftRPCClient::onConnection, this, _1));
}

void RaftRPCClient::onConnection(const tmuduo::net::TcpConnectionPtr &conn) {
    LOG_DEBUG("stub_ init");
    stub_ = new RaftRPC::Stub(std::any_cast<std::shared_ptr<RpcChannel>>(conn->getContext()).get());
}

void RaftRPCClient::onVoteResponse(RequestVoteReply* response) {
    LOG_DEBUG("Get vote response");
    delete(response);
}

void RaftRPCClient::onAppendResponse(RequestAppendReply* response) {
    LOG_DEBUG("Get append response");
    delete(response);
}

void RaftRPCClient::voteRequest() {
    RequestVoteArgs request;
    RequestVoteReply* response = new RequestVoteReply;
    stub_->RequestVote(nullptr, &request, response, NewCallback(this, &RaftRPCClient::onVoteResponse, response));
}

void RaftRPCClient::appendRequest() {
    RequestAppendArgs request;
    RequestAppendReply* response = new RequestAppendReply;
    stub_->RequestAppend(nullptr, &request, response, NewCallback(this, &RaftRPCClient::onAppendResponse, response));
}

void RaftRPCClient::makeVoteRequest() {
    loop_->runInLoop(std::bind(&RaftRPCClient::voteRequest, this));
}

void RaftRPCClient::makeAppendRequest() {
    loop_->runInLoop(std::bind(&RaftRPCClient::appendRequest, this));
}

