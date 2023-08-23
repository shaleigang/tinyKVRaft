//
// Created by slg on 8/10/23.
//

#include "kvstore.h"

#include <condition_variable>

using namespace kv;

CommandRPCImp::CommandRPCImp(raft::Raft *raft, kvstore* kv)
  : raft_(raft),
    kvstore_(kv) { }

void CommandRPCImp::Command(::google::protobuf::RpcController *controller,
                            const ::kv::KvCommand *request,
                            ::kv::KvCommandReply *response,
                            ::google::protobuf::Closure *done) {
    if(!raft_->isLeader()) {
        response->set_leader(false);
    }
    else {
        response->set_leader(true);
        auto it = kvstore_->latest_applied_seq_per_client_.find(request->cid());
        if(it != kvstore_->latest_applied_seq_per_client_.end() && request->seq() <= it->second) {
            if(request->operation() == Ops::GET) {
                std::string ret;
                bool success = kvstore_->store_.searchNode(request->key(), ret);
                if(success) {
                    response->set_value(ret);
                    response->set_error(Error::ERROR_OK);
                }
                else {
                    response->set_error(Error::ERROR_NO_KEY);
                }
            }
        }
        else {
            std::string cmd;
            request->SerializeToString(&cmd);
            uint32_t term;
            uint32_t index;
            kvstore_->raft_.start(cmd, index, term);

            std::condition_variable cond;
            kvstore_->notify_[index] = &cond;
            cond.wait();



        }
    }
}

void kvstore::applyFunc(uint32_t server_id, raft::LogEntry) {

}