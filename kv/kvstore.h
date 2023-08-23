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
namespace kv {

class kvstore : public tmuduo::noncopyable {
public:

private:
    SkipList<int, int> store_;

    tmuduo::net::EventLoop* loop_;
    raft::Raft raft_;

    tmuduo::net::RpcServer commandServer_;
};

}


#endif //TINYKVRAFT_KVSTORE_H
