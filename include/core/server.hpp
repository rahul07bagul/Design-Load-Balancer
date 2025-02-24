#pragma once
#include <string>
#include <cstdint>
#include <atomic>
#include <chrono>
#include "core/process/process.hpp"
#include <iostream>

class Server {
public:
    Server(const std::string& host, uint16_t port);
    std::string getAddress() const;
    uint16_t getPort() const;
    std::string getId() const;
    bool isHealthy() const;
    void setHealthStatus(bool status);
    std::chrono::system_clock::time_point getLastHealthCheckTime() const;
    void setLastHealthCheckTime(std::chrono::system_clock::time_point t);
    
    uint64_t getRequestCount() const { return request_count_.load(); }
    void incrementRequestCount() { request_count_.fetch_add(1); }
    
    void incrementActiveConnections();
    void decrementActiveConnections();
    uint64_t getActiveConnections() const;

    void setProcess(std::unique_ptr<Process> proc) { process_ = std::move(proc); }
    Process* getProcess() const { return process_.get(); }

private:
    std::string host_;
    uint16_t port_;
    bool is_healthy_;
    std::string id_;
    std::chrono::system_clock::time_point last_health_check_time_;
    std::atomic<uint64_t> request_count_{0};
    std::atomic<uint64_t> active_connections_{0};
    std::unique_ptr<Process> process_;
};