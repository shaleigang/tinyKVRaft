//
// Created by slg on 8/7/23.
//

#include "RaftRPCImp.h"
#include "Raft.h"
#include <tinyMuduo/base/Logger.h>

#include <utility>

using tmuduo::Logger;
using namespace raft;

RaftRPCImp::RaftRPCImp(raft::RaftRPCImp::RPCVoteCallback onRequestVoteCallback,
                       raft::RaftRPCImp::RPCAppendCallback onRequestAppendCallback)
  : onRequestVoteCallback_(onRequestVoteCallback),
    onRequestAppendCallback_(onRequestAppendCallback) {

}


void RaftRPCImp::RequestVote(::google::protobuf::RpcController* controller,
                             const ::raft::RequestVoteArgs* request,
                             ::raft::RequestVoteReply* response,
                             ::google::protobuf::Closure* done) {
    LOG_INFO("RaftRPC::RequestVote");

    onRequestVoteCallback_(request, response);

    if(done) {
        done->Run();
    }
}

void RaftRPCImp::RequestAppend(::google::protobuf::RpcController* controller,
                               const ::raft::RequestAppendArgs* request,
                               ::raft::RequestAppendReply* response,
                               ::google::protobuf::Closure* done) {
    LOG_INFO("RaftRPC::RequestAppend");

    onRequestAppendCallback_(request, response);

    if(done) {
        done->Run();
    }
}