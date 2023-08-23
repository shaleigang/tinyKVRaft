//
// Created by slg on 8/10/23.
//

#ifndef TINYKVRAFT_KVSTORE_H
#define TINYKVRAFT_KVSTORE_H

#include <tinyMuduo/base/noncopyable.h>
#include <tinyMuduo/net/EventLoop.h>
#include <tinyMuduo/net/rpc/RpcServer.h>
#include "SkipList.h"
#include "raft/Raft.h"
#include "command.pb.h"


namespace kv {

class kvstore;

class CommandRPCImp : public CommandRPC {
public:
    CommandRPCImp(raft::Raft* raft, kvstore* kv);

    void Command(::google::protobuf::RpcController* controller,
                 const ::kv::KvCommand* request,
                 ::kv::KvCommandReply* response,
                 ::google::protobuf::Closure* done) override;

private:
    raft::Raft* raft_;
    kvstore* kvstore_;
};

class kvstore : public tmuduo::noncopyable {
public:
    kvstore();

private:
    void applyFunc(uint32_t server_id, raft::LogEntry);

private:
    friend CommandRPCImp;

    uint32_t myId_;
    SkipList<std::string, std::string> store_;

    tmuduo::net::EventLoop* loop_;
    raft::Raft raft_;

    tmuduo::net::RpcServer commandServer_;
    CommandRPCImp commandRpcImp_;

    std::unordered_map<uint32_t, std::condition_variable*> notify_;
    std::unordered_map<int64_t, uint32_t> latest_applied_seq_per_client_;
};

}


#endif //TINYKVRAFT_KVSTORE_H
