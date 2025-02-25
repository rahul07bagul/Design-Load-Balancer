#include "core/server_manager.hpp"
#include <windows.h>
#include <process.h>
#include <stdexcept>
#include <string>
#include <vector>

static HANDLE CreateServerProcess(const std::string& command);

ServerManager::ServerManager(const std::string& executable_path,
                           uint16_t start_port,
                           size_t min_servers,
                           size_t max_servers)
    : executable_path_(executable_path)
    , next_port_(start_port)
    , min_servers_(min_servers)
    , max_servers_(max_servers) {
    
    for (size_t i = 0; i < min_servers_; ++i) {
        addServer();
    }
}

std::vector<std::shared_ptr<Server>> ServerManager::getAllServers() {
    std::lock_guard<std::mutex> lock(mutex_);
    return servers_;
}

std::shared_ptr<Server> ServerManager::findServerById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& srv : servers_) {
        if (srv->getId() == id) {
            return srv;
        }
    }
    return nullptr;
}

bool ServerManager::removeServerById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (active_servers <= min_servers_) {
        return false;
    }
    for (auto it = servers_.begin(); it != servers_.end(); ++it) {
        if ((*it)->getId() == id) {
            if((*it)->getProcess() != nullptr) {
                (*it)->getProcess()->terminate();
            }
            (*it)->setHealthStatus(false);
            active_servers--;
            return true;
        }
    }
    return false;
}

std::shared_ptr<Server> ServerManager::addServer() {
    if (active_servers >= max_servers_) {
        return nullptr;
    }
    auto server = std::make_shared<Server>("127.0.0.1", next_port_);
    std::string command = executable_path_ + " " + std::to_string(next_port_);
    auto process = ProcessFactory::createProcess();
    if (!process->start(command)) {
        return nullptr;
    }
    server->setProcess(std::move(process));
    servers_.push_back(server);
    active_servers++;
    next_port_++;
    return server;
}

void ServerManager::updateServerHealth(const std::string& id, 
    bool isHealthy, 
    double cpuUsage, 
    double memoryUsage) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto server = findServerById(id);
    if (!server) {
        return;
    }

    // Update the server object
    server->setHealthStatus(isHealthy);
    server->setCPUUsage(cpuUsage);
    server->setMemoryUsage(memoryUsage);
}

std::vector<std::shared_ptr<Server>> ServerManager::getActiveServers() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<Server>> active_servers;
    for (const auto& srv : servers_) {
        if (srv->isHealthy()) {
            active_servers.push_back(srv);
        }
    }
    return active_servers;
}

ServerManager::ServerStats ServerManager::getServerStats() {
    std::lock_guard<std::mutex> lock(mutex_);
    ServerStats stats;
    stats.total_servers = servers_.size();
    stats.active_servers = active_servers;
    stats.inactive_servers = servers_.size() - active_servers;
    stats.min_servers = min_servers_;
    stats.max_servers = max_servers_;
    return stats;
}