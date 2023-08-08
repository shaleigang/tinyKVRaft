//
// Created by slg on 8/7/23.
//

#ifndef TINYKVRAFT_RAFTRPCIMP_H
#define TINYKVRAFT_RAFTRPCIMP_H

#include "raftRPC.pb.h"

namespace raft {

class Raft;

class RaftRPCImp : public RaftRPC {
public:
    typedef std::function<void(const RequestVoteArgs*, RequestVoteReply*)> RPCVoteCallback;
    typedef std::function<void(const RequestAppendArgs*, RequestAppendReply*)> RPCAppendCallback;

    RaftRPCImp() = default;
    RaftRPCImp(RPCVoteCallback, RPCAppendCallback);

    void RequestVote(::google::protobuf::RpcController* controller,
                     const ::raft::RequestVoteArgs* request,
                     ::raft::RequestVoteReply* response,
                     ::google::protobuf::Closure* done) override;
    void RequestAppend(::google::protobuf::RpcController* controller,
                       const ::raft::RequestAppendArgs* request,
                       ::raft::RequestAppendReply* response,
                       ::google::protobuf::Closure* done) override;

    void setVoteCallback(const RPCVoteCallback& cb) { onRequestVoteCallback_ = cb; }
    void setAppendCallback(const RPCAppendCallback& cb) { onRequestAppendCallback_ = cb; }

private:
    RPCVoteCallback onRequestVoteCallback_;
    RPCAppendCallback onRequestAppendCallback_;
};

}


#endif //TINYKVRAFT_RAFTRPCIMP_H
