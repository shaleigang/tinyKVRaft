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
    RaftRPCImp() = default;
    RaftRPCImp(Raft* raft);

    // TODO: COMPLETE THIS
    void RequestVote(::google::protobuf::RpcController* controller,
                     const ::raft::RequestVoteArgs* request,
                     ::raft::RequestVoteReply* response,
                     ::google::protobuf::Closure* done) override;
    void RequestAppend(::google::protobuf::RpcController* controller,
                       const ::raft::RequestAppendArgs* request,
                       ::raft::RequestAppendReply* response,
                       ::google::protobuf::Closure* done) override;

private:
    Raft* raft_;
};

}


#endif //TINYKVRAFT_RAFTRPCIMP_H
