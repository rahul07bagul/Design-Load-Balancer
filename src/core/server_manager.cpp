#include "core/server_manager.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

ServerManager::ServerManager(const std::string& executable_path,
                           uint16_t start_port,
                           size_t min_servers,
                           size_t max_servers)
    : executable_path_(executable_path)
    , next_port_(start_port)
    , min_servers_(min_servers)
    , max_servers_(max_servers) {
    
    // Start minimum number of servers
    for (size_t i = 0; i < min_servers_; ++i) {
        addServer();
    }
}

bool ServerManager::addServer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (servers_.size() >= max_servers_) {
        return false;
    }

    auto server = std::make_shared<Server>("localhost", next_port_);
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        std::string port_str = std::to_string(next_port_);
        execl(executable_path_.c_str(), executable_path_.c_str(), 
              port_str.c_str(), nullptr);
        exit(1);
    } else if (pid > 0) {
        servers_.push_back(server);
        next_port_++;
        return true;
    }
    
    return false;
}

bool ServerManager::removeServer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (servers_.size() <= min_servers_) {
        return false;
    }

    servers_.pop_back();
    return true;
}

std::vector<std::shared_ptr<Server>> ServerManager::getActiveServers() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Server>> active_servers;
    for (const auto& server : servers_) {
        if (server->isHealthy()) {
            active_servers.push_back(server);
        }
    }
    return active_servers;
}