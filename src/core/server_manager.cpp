#include "core/server_manager.hpp"
#include <windows.h>
#include <process.h>
#include <stdexcept>
#include <string>
#include <vector>

// Forward declaration
static HANDLE CreateServerProcess(const std::string& command);

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

std::shared_ptr<Server> ServerManager::addServerAndReturn() {
    // Similar to addServer() logic, but returns the server
    if (servers_.size() >= max_servers_) {
        return nullptr;
    }
    auto server = std::make_shared<Server>("127.0.0.1", next_port_++);
    std::string command = executable_path_ + " " + std::to_string(next_port_);
    HANDLE hProcess = CreateServerProcess(command);
    if (hProcess == NULL) {
        return nullptr;
    }
    servers_.push_back(server);
    return server;
}

bool ServerManager::removeServerById(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (servers_.size() <= min_servers_) {
        return false;
    }
    for (auto it = servers_.begin(); it != servers_.end(); ++it) {
        if ((*it)->getId() == id) {
            servers_.erase(it);
            return true;
        }
    }
    return false;
}

bool ServerManager::addServer() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (servers_.size() >= max_servers_) {
        return false;
    }

    auto server = std::make_shared<Server>("127.0.0.1", next_port_);
    
    std::string command = executable_path_ + " " + std::to_string(next_port_);
    HANDLE hProcess = CreateServerProcess(command);
    if (hProcess != NULL) {
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

static HANDLE CreateServerProcess(const std::string& command) {
    STARTUPINFOW siw;
    PROCESS_INFORMATION pi;

    ZeroMemory(&siw, sizeof(siw));
    siw.cb = sizeof(siw);
    ZeroMemory(&pi, sizeof(pi));

    // Convert std::string command to std::wstring
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, NULL, 0);
    std::wstring wCommand(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, command.c_str(), -1, &wCommand[0], size_needed);

    if (!CreateProcessW(nullptr,      // No module name (use command line)
        &wCommand[0],                 // Command line as wide string
        nullptr,                      // Process handle not inheritable
        nullptr,                      // Thread handle not inheritable
        FALSE,                        // Set handle inheritance to FALSE
        0,                           // No creation flags
        nullptr,                     // Use parent's environment block
        nullptr,                     // Use parent's starting directory 
        &siw,                        // Pointer to STARTUPINFOW structure
        &pi)) {                      // Pointer to PROCESS_INFORMATION structure
        throw std::runtime_error("CreateProcess failed");
    }

    CloseHandle(pi.hThread);
    return pi.hProcess;
}