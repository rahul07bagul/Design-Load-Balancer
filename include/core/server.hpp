#pragma once
#include <string>
#include <cstdint>
#include <atomic>
#include <chrono>
#include "core/process/process.hpp"
#include <iostream>

class Server {
public:
    Server(const std::string& host, int port);
    std::string getAddress() const;
    int getPort() const;
    std::string getId() const;
    bool isHealthy() const;
    void setHealthStatus(bool status);

    double getCPUUsage() const;
    void setCPUUsage(double usage);
    double getMemoryUsage() const;
    void setMemoryUsage(double usage);
    
    std::chrono::system_clock::time_point getLastHealthCheckTime() const;
    void setLastHealthCheckTime(std::chrono::system_clock::time_point t);
    
    int getRequestCount() const { return request_count_.load(); }
    void incrementRequestCount() { request_count_.fetch_add(1); }
    
    void incrementActiveConnections();
    void decrementActiveConnections();
    int getActiveConnections() const;

    void setProcess(std::unique_ptr<Process> proc) { process_ = std::move(proc); }
    Process* getProcess() const { return process_.get(); }

private:
    std::string host_;
    int port_;
    bool is_healthy_;
    std::string id_;
    std::chrono::system_clock::time_point last_health_check_time_;
    std::atomic<int> request_count_{0};
    std::atomic<int> active_connections_{0};
    std::unique_ptr<Process> process_;
    double cpu_usage;
    double memory_usage;
};