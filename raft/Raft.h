//
// Created by slg on 8/5/23.
//

#ifndef TINYKVRAFT_RAFT_H
#define TINYKVRAFT_RAFT_H

#include <utility>
#include <vector>
#include <tinyMuduo/net/EventLoop.h>
#include <tinyMuduo/net/rpc/RpcServer.h>

#include "RaftRPCClient.h"

namespace raft {

class RaftRPCImp;

using tmuduo::net::TimerId;

typedef std::function<void(uint32_t, LogEntry)> ApplyFunc;

class Raft {
public:
    enum State {
        Follower = 0,
        Candidate,
        Leader,
    };

    Raft(tmuduo::net::EventLoop* loop,
         const tmuduo::net::InetAddress& serverAddr,
         const std::vector<RaftRPCClientPtr>& peers,
         uint32_t myId);
    ~Raft();

    void start();
    bool start(const std::string& cmd, uint32_t& index, uint32_t& term);
    void quit();

    bool isLeader() { return state_ == Leader; }
    uint32_t term() { return currentTerm_; }
    tmuduo::net::RpcServer& getRpcServer() { return server_; }
    void setApplyFunc(ApplyFunc&& func) { applyFunc_ = std::move(func); }

private:
    void resetLeaderState();
    void turnToFollower(uint32_t term);
    void turnToCandidate();
    void turnToLeader();
    void CandidateRequestVote();
    void heartbeat();

    double getElectionTimeout();

    uint32_t getLastEntryIndex() const { return logs_.size() - 1; }
    const LogEntry& getLogEntryAt(uint32_t index) const {return logs_[index]; };
    bool thisIsMoreUpToDate(uint32_t last_log_index, uint32_t last_log_term) const;
    void constructLog(size_t next_index, std::shared_ptr<RequestAppendArgs> append_args);
    void applyLogs();
    static void defaultApplyFunc(uint32_t server_id, LogEntry);
    void updateCommitIndex();

private:
    tmuduo::net::EventLoop* loop_;
    tmuduo::net::RpcServer server_;
    RaftRPCImp* raftRPCService_;
    std::vector<RaftRPCClientPtr> peers_;

    State state_;
    uint32_t myId_;

    uint32_t currentTerm_;
    uint32_t votedFor_;
    uint32_t votedGain_;
    std::vector<LogEntry> logs_;

    uint32_t commitIndex_;
    uint32_t lastApplied_;

    std::vector<uint32_t> nextIndex_;
    std::vector<uint32_t> matchIndex_;

    int heartbeatInterval_;
    TimerId timeoutId_;
    TimerId heartbeatId_;

    bool running_;

    ApplyFunc applyFunc_;

};

}


#endif //TINYKVRAFT_RAFT_H
