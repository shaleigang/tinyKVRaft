//
// Created by slg on 8/7/23.
//

#include "RaftRPCImp.h"
#include <tinyMuduo/base/Logger.h>

using tmuduo::Logger;
using namespace raft;

RaftRPCImp::RaftRPCImp(Raft* raft)
  : raft_(raft){

}

void RaftRPCImp::RequestVote(::google::protobuf::RpcController* controller,
                             const ::raft::RequestVoteArgs* request,
                             ::raft::RequestVoteReply* response,
                             ::google::protobuf::Closure* done) {
    LOG_INFO("RaftRPC::RequestVote");
    response->set_term(123);
    response->set_votegranted(true);
    if(done) {
        done->Run();
    }
}

void RaftRPCImp::RequestAppend(::google::protobuf::RpcController* controller,
                               const ::raft::RequestAppendArgs* request,
                               ::raft::RequestAppendReply* response,
                               ::google::protobuf::Closure* done) {
    LOG_INFO("RaftRPC::RequestAppend");
    response->set_term(123);
    response->set_success(true);
    if(done) {
        done->Run();
    }
}