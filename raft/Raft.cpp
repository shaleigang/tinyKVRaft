//
// Created by slg on 8/5/23.
//

#include "Raft.h"
#include "RaftRPCImp.h"

#include <tinyMuduo/base/Logger.h>

using namespace raft;
using tmuduo::Logger;

Raft::Raft(tmuduo::net::EventLoop *loop,
           const tmuduo::net::InetAddress &serverAddr,
           const std::vector<RaftRPCClientPtr> &peers,
           uint32_t myId)
  : loop_(loop),
    server_(loop, serverAddr),
    peers_(peers),
    state_(Follower),
    myId_(myId),
    currentTerm_(0),
    votedFor_(-1),
    votedGain_(0),
    commitIndex_(0),
    lastApplied_(0),
    heartbeatInterval_(0.1),
    running_(false),
    applyFunc_(std::bind(&Raft::defaultApplyFunc,
                         std::placeholders::_1, std::placeholders::_2)){

    raftRPCService_ = new RaftRPCImp(this);
    server_.registerService(raftRPCService_);
    server_.start();

    LogEntry entry;
    entry.set_term(0);
    entry.set_index(0);
    logs_.push_back(entry);

    resetLeaderState();
}

Raft::~Raft() {
    LOG_DEBUG("~Raft()");
}

void Raft::defaultApplyFunc(uint32_t server_id, raft::LogEntry entry) {
    LOG_DEBUG("apply msg index = %d", entry.index());
}

double Raft::getElectionTimeout() {
    return double(300 + rand() % 201) / 1000;
}

void Raft::resetLeaderState() {
    nextIndex_.clear();
    matchIndex_.clear();
    nextIndex_.insert(nextIndex_.begin(), peers_.size(), logs_.size());
    matchIndex_.insert(matchIndex_.begin(), peers_.size(), 0);
}

void Raft::start() {
    turnToFollower(0);
}

bool Raft::start(const std::string &cmd, uint32_t &index, uint32_t &term) {
    bool is_leader = state_ == Leader;
    term = currentTerm_;
    if (is_leader) {
        index = getLastEntryIndex() + 1;
        LogEntry entry;
        entry.set_term(term);
        entry.set_index(index);
        entry.set_command(cmd);
        logs_.push_back(entry);
        LOG_DEBUG(" :receive new command %s", cmd.c_str());
    }
    return is_leader;
}

void Raft::quit() {
    running_ = false;
    LOG_DEBUG("Raft quit");
}

void Raft::turnToFollower(uint32_t term) {
    if(state_ == Leader) {
        loop_->cancel(heartbeatId_);
        LOG_DEBUG("cancel heartbeat_id");
    }
    else {
        loop_->cancel(timeoutId_);
        LOG_DEBUG("cancel timeout_id");
    }
    state_ = Follower;
    LOG_DEBUG("become Follower");
    currentTerm_ = term;
    votedFor_ = -1;
    votedGain_ = 0;
    timeoutId_ = loop_->runAfter(getElectionTimeout(), std::bind(&Raft::turnToCandidate, this));
}

void Raft::turnToCandidate() {
    loop_->cancel(timeoutId_);
    LOG_DEBUG("cancel timeout_id");

    state_ = Candidate;
    currentTerm_ += 1;
    votedFor_ = myId_;
    votedGain_ = 1;
    LOG_DEBUG("become Candidate");
    CandidateRequestVote();
    timeoutId_ = loop_->runAfter(getElectionTimeout(), std::bind(&Raft::turnToCandidate, this));
}

void Raft::turnToLeader() {
    loop_->cancel(timeoutId_);
    LOG_DEBUG("cancel timeout_id");
    state_ = Leader;
    LOG_DEBUG("become Leader");
    resetLeaderState();
    heartbeat();
    heartbeatId_ = loop_->runEvery(heartbeatInterval_,
                                   std::bind(&Raft::heartbeat, this));
    LOG_DEBUG("start heartbeat");
}

void Raft::CandidateRequestVote() {
    if (state_ != Candidate) {
        return;
    }

    for (int i = 0; i < peers_.size(); ++i) {
        if(i != myId_) {
            peers_[i]->makeVoteRequest();
        }
    }
}

void Raft::heartbeat() {
    if (state_ != Leader) {
        return;
    }

    for (int i = 0; i < peers_.size(); ++i) {
        if(i != myId_) {
            // TODO: FIX THIS
            peers_[i]->makeAppendRequest();
        }
    }
}

bool Raft::thisIsMoreUpToDate(uint32_t last_log_index, uint32_t last_log_term) const {
    uint32_t this_last_log_index = getLastEntryIndex();
    uint32_t this_last_log_term = logs_[this_last_log_index].term();
    return (this_last_log_term > last_log_term
            || (this_last_log_term == last_log_term && this_last_log_index > last_log_index));
}

void Raft::constructLog(size_t next_index, std::shared_ptr<RequestAppendArgs> append_args) {
    for (size_t i = next_index; i < logs_.size(); ++i) {
        const LogEntry& originEntry = logs_[i];
        LogEntry* entry = append_args->add_entries();
        entry->set_term(originEntry.term());
        entry->set_index(originEntry.index());
        entry->set_command(originEntry.command());
    }
}

void Raft::applyLogs() {
    while (lastApplied_ < commitIndex_) {
        lastApplied_++;
        applyFunc_(myId_, getLogEntryAt(lastApplied_));
    }
}

void Raft::updateCommitIndex() {
    std::vector<uint32_t> match_index(matchIndex_);
    match_index[myId_] = getLastEntryIndex();

    std::sort(match_index.begin(), match_index.end());
    uint32_t N = match_index[match_index.size() / 2];

    if (state_ == Leader && N > commitIndex_ && logs_[N].term() == currentTerm_) {
        commitIndex_ = N;
        applyLogs();
    }
}












