//
// Created by slg on 8/5/23.
//

#include "Raft.h"

#include <tinyMuduo/base/Logger.h>

using namespace raft;
using tmuduo::Logger;
using std::placeholders::_1;
using std::placeholders::_2;

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
    applyFunc_(std::bind(&Raft::defaultApplyFunc, _1, _2)){

    raftRPCService_ = new RaftRPCImp(std::bind(&Raft::onRequestVote, this, _1, _2),
                                     std::bind(&Raft::onRequestAppend, this, _1, _2));
    server_.registerService(raftRPCService_);
    server_.start();

    LogEntry entry;
    entry.set_term(0);
    entry.set_index(0);
    logs_.push_back(entry);

    resetLeaderState();

    for(int i = 0; i < peers_.size(); ++i) {
        if(i != myId_) {
            peers_[i]->setVoteReplyCallback(std::bind(&Raft::onVoteReply, this, _1));
            peers_[i]->setAppendReplyCallback(std::bind(&Raft::onAppendReply, this, _1, _2));
        }
    }

    for(int i = 0; i < peers_.size(); ++i) {
        if(i != myId_) {
            peers_[i]->connect();
        }
    }
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
            RequestVoteArgsPtr request = std::make_shared<RequestVoteArgs>();
            request->set_term(currentTerm_);
            request->set_candidateid(myId_);
            request->set_lastlogterm(getLogEntryAt(getLastEntryIndex()).term());
            request->set_lastlogindex(getLastEntryIndex());
            peers_[i]->makeVoteRequest(request);
        }
    }
}

void Raft::heartbeat() {
    if (state_ != Leader) {
        return;
    }

    for (int i = 0; i < peers_.size(); ++i) {
        if(i != myId_) {
            RequestAppendArgsPtr request = std::make_shared<RequestAppendArgs>();
            request->set_term(currentTerm_);
            request->set_leaderid(myId_);
            int nextIndex = nextIndex_[i];
            request->set_prevlogindex(nextIndex - 1);
            request->set_prevlogterm(logs_[nextIndex - 1].term());
            request->set_leadercommit(commitIndex_);
            peers_[i]->makeAppendRequest(i, request);
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

void Raft::onRequestVote(const raft::RequestVoteArgs *request, raft::RequestVoteReply *response) {
    if (request->term() < currentTerm_) {
        response->set_term(currentTerm_);
        response->set_votegranted(false);
        LOG_DEBUG("vote reject %d", request->candidateid());
    }
    else {
        response->set_term(currentTerm_);
        if (votedFor_ == -1 && !thisIsMoreUpToDate(request->lastlogindex(), request->lastlogterm())) {
            votedFor_ = request->candidateid();
            response->set_votegranted(true);
            LOG_DEBUG("vote for %d", request->candidateid());
        }
        else {
            response->set_votegranted(false);
            LOG_DEBUG("vote reject %d", request->candidateid());
        }
        if (request->term() > currentTerm_) {
            turnToFollower(request->term());
        }
    }
}

void Raft::onRequestAppend(const raft::RequestAppendArgs *request, raft::RequestAppendReply *response) {
    LOG_DEBUG("get log append from %d", request->leaderid());
    response->set_term(currentTerm_);

    if(request->term() < currentTerm_) {
        response->set_success(false);
        response->set_conflictindex(0);
        response->set_conflictindex(0);
    }
    else {
        turnToFollower(request->term());

        uint32_t preLogIndex = request->prevlogindex();
        if((preLogIndex == 0) ||
        (preLogIndex <= getLastEntryIndex() && logs_[preLogIndex].term() == request->prevlogterm())) {
            response->set_success(true);
            response->set_conflictindex(0);
            response->set_conflictterm(0);

            if (request->entries_size() > 0) {
                logs_.erase(logs_.begin() + preLogIndex + 1, logs_.end());

                std::vector<LogEntry> newLogs_;
                for (int i = 0; i < request->entries_size(); ++i) {
                    const LogEntry &entry = request->entries(i);
                    newLogs_.push_back(entry);
                }
                const LogEntry &lastEntry = request->entries(request->entries_size() - 1);
                uint32_t index = preLogIndex + request->entries_size() + 1;
                if (index <= getLastEntryIndex() && logs_[index].term() >= lastEntry.term()) {
                    for (uint32_t i = index; i < logs_.size(); ++i) {
                        newLogs_.push_back(logs_[i]);
                    }
                }
                logs_.insert(logs_.end(), newLogs_.begin(), newLogs_.end());
            }

            if (request->leadercommit() > commitIndex_) {
                commitIndex_ = std::min(request->leadercommit(), getLastEntryIndex());
                applyLogs();
            }
        }
        else {
            response->set_success(false);
            uint32_t preLogIndex = request->prevlogindex();
            if (preLogIndex > getLastEntryIndex()) {
                response->set_conflictindex(logs_.size());
                response->set_conflictterm(0);
            }
            else {
                response->set_conflictterm(getLogEntryAt(preLogIndex).term());
                uint32_t i = preLogIndex;
                while (i > 0 && getLogEntryAt(i).term() == response->conflictterm()) {
                    --i;
                }
                response->set_conflictindex(i + 1);
            }
        }
    }
}

void Raft::onVoteReply(raft::RequestVoteReply *response) {
    if(currentTerm_ < response->term()) {
        LOG_DEBUG("failed in election, become follower");
        turnToFollower(response->term());
    }
    else if (response->votegranted() && state_ == Candidate && response->term() == currentTerm_) {
        ++votedGain_;
        if(votedGain_ > (peers_.size() / 2)) {
            turnToLeader();
        }
    }
}

RequestAppendArgsPtr Raft::onAppendReply(RaftRPCClient::AppendReplyCallbackMsg msg, raft::RequestAppendReply *response) {
    uint32_t targetServer = msg.first;
    RequestAppendArgsPtr request = msg.second;
    if(state_ != Leader && currentTerm_ != request->term()) {
        return nullptr;
    }
    if(response->success()) {
        matchIndex_[targetServer] = request->prevlogindex() + request->entries_size();
        nextIndex_[targetServer] = matchIndex_[targetServer] + 1;
        LOG_DEBUG("update next_index[%d] to %d", targetServer, nextIndex_[targetServer]);
        updateCommitIndex();
        return nullptr;
    }
    else {
        if (request->term() < response->term()) {
            nextIndex_[targetServer] = response->conflictindex();
            return nullptr;
        }
        else {
            if (response->conflictterm() == 0) {
                nextIndex_[targetServer] = response->conflictindex();
            }
            else {
                uint32_t preLogIndex = request->prevlogindex();
                while ((preLogIndex >= response->conflictindex()) && logs_[preLogIndex].term() != response->conflictterm()) {
                    --preLogIndex;
                }
                nextIndex_[targetServer] = preLogIndex + 1;
            }
            uint32_t nextIndex = nextIndex_[targetServer];
            RequestAppendArgsPtr newRequest = std::make_shared<RequestAppendArgs>();
            newRequest->set_term(currentTerm_);
            newRequest->set_leaderid(myId_);
            newRequest->set_prevlogindex(nextIndex - 1);
            newRequest->set_prevlogterm(logs_[nextIndex - 1].term());
            newRequest->set_leadercommit(commitIndex_);
            LOG_DEBUG("resend append rpc to %d change pre_log_index to ", targetServer, nextIndex - 1);
            return newRequest;
        }
    }
}












