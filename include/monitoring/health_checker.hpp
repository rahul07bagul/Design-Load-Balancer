#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "core/server.hpp"
#include "core/server_manager.hpp"

#pragma comment(lib, "Ws2_32.lib")

class HealthChecker {
public:
    explicit HealthChecker(std::shared_ptr<ServerManager> server_manager);
    ~HealthChecker();

    // Non-copyable
    HealthChecker(const HealthChecker&) = delete;
    HealthChecker& operator=(const HealthChecker&) = delete;

    void start();
    void stop();

private:
    void checkHealth();
    bool isServerResponding(const std::shared_ptr<Server>& server);
    bool initializeWinSock();

    std::shared_ptr<ServerManager> server_manager_;
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> health_check_thread_;
    const std::chrono::seconds check_interval_{30}; // 30 seconds interval
    bool wsaInitialized_;
};