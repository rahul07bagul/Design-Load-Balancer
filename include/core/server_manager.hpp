#pragma once
#include <vector>
#include <memory>
#include <mutex>
#include <iostream>
#include "core/server.hpp"
#include "core/process/process_factory.hpp"

class ServerManager {
public:
    ServerManager(const std::string& executable_path, 
                 uint16_t start_port,
                 size_t min_servers,
                 size_t max_servers);
    
    std::vector<std::shared_ptr<Server>> getAllServers();
    std::shared_ptr<Server> findServerById(const std::string& id);
    void updateServerHealth(const std::string& id, bool health, double cpu_usage, double memory_usage);
    bool removeServerById(const std::string& id);
    std::shared_ptr<Server> addServer();
    std::vector<std::shared_ptr<Server>> getActiveServers();
    struct ServerStats {
        size_t total_servers;
        size_t active_servers;
        size_t inactive_servers;
        size_t min_servers;
        size_t max_servers;
    };
    ServerStats getServerStats();
    
private:
    std::string executable_path_;
    uint16_t next_port_;
    size_t min_servers_;
    size_t max_servers_;
    std::atomic<size_t> active_servers;
    std::vector<std::shared_ptr<Server>> servers_;
    std::mutex mutex_;
};