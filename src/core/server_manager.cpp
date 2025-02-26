#include "core/server_manager.hpp"
#include <windows.h>
#include <process.h>
#include "utils/config.hpp"
#include <stdexcept>
#include <string>
#include <vector>

static HANDLE CreateServerProcess(const std::string& command);

ServerManager::ServerManager(const std::string& executable_path, int start_port, size_t min_servers, size_t max_servers)
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
    //std::lock_guard<std::mutex> lock(mutex_);
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
    auto server = std::make_shared<Server>(server_address, next_port_);
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

// int ServerManager::findAvailablePort() {
//     // If we have available ports in our pool, use one
//     if (!available_ports_.empty()) {
//         int port = *available_ports_.begin();
//         available_ports_.erase(available_ports_.begin());
//         return port;
//     }
    
//     // Otherwise, use the next_port_ if it's still within range
//     if (next_port_ < start_port_ + max_port_range_) {
//         int port = next_port_++;
//         return port;
//     }
    
//     return -1;
// }

void ServerManager::updateServerHealth(const std::string& id, bool isHealthy, double cpuUsage, double memoryUsage) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto server = findServerById(id);
    if (!server) {
        return;
    }
    
    //If a server dies unexpectedly (not through the health checker), 
    //the active_servers counter in ServerManager can become inconsistent with the actual number of healthy servers
    if(isHealthy && !server->isHealthy()) {
        //it should never be the case that a server is unhealthy and not in the servers_ list, 
        //as the server which goes offline never comes back online on same port,
        //but we should check for it anyway
        active_servers++;
    } else if (!isHealthy && server->isHealthy()) {
        active_servers--;
    }
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