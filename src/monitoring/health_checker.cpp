#include "monitoring/health_checker.hpp"
#include <iostream>

HealthChecker::HealthChecker(std::shared_ptr<ServerManager> server_manager)
    : server_manager_(server_manager)
    , running_(false)
    , wsaInitialized_(false) {
    initializeWinSock();
}

HealthChecker::~HealthChecker() {
    stop();
    if (wsaInitialized_) {
        WSACleanup();
    }
}

bool HealthChecker::initializeWinSock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    wsaInitialized_ = true;
    return true;
}

void HealthChecker::start() {
    bool expected = false;
    if (running_.compare_exchange_strong(expected, true)) {
        health_check_thread_ = std::make_unique<std::thread>(&HealthChecker::checkHealth, this);
    }
}

void HealthChecker::stop() {
    bool expected = true;
    if (running_.compare_exchange_strong(expected, false) && health_check_thread_) {
        if (health_check_thread_->joinable()) {
            health_check_thread_->join();
        }
    }
}

void HealthChecker::checkHealth() {
    while (running_) {
        std::cout << "\n=== Health Check Started ===" << std::endl;
        auto servers = server_manager_->getActiveServers();
        
        for (const auto& server : servers) {
            bool was_healthy = server->isHealthy();
            bool is_healthy = isServerResponding(server);
            
            std::cout << "Checking server " << server->getId() << ":\n"
                     << "  Status: " << (is_healthy ? "Healthy" : "Unhealthy") << std::endl;
            
            if (was_healthy && !is_healthy) {
                std::cout << "Server " << server->getId() << " is down" << std::endl;
                server->setHealthStatus(false);
                
                if (server_manager_->addServer()) {
                    std::cout << "Created new server to replace " << server->getId() << std::endl;
                }
            } else if (!was_healthy && is_healthy) {
                std::cout << "Server " << server->getId() << " is back online" << std::endl;
                server->setHealthStatus(true);
            }
        }
        std::cout << "=== Health Check Completed ===" << std::endl;

        std::this_thread::sleep_for(check_interval_);
    }
}

bool HealthChecker::isServerResponding(const std::shared_ptr<Server>& server) {
    if (!wsaInitialized_) {
        std::cerr << "WinSock not initialized" << std::endl;
        return false;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set socket timeout
    DWORD timeout = 5000; // Increased to 5 seconds
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    // Set non-blocking mode
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return false;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server->getPort());
    
    // Convert address with error checking
    if (inet_pton(AF_INET, server->getAddress().c_str(), &server_addr.sin_addr) != 1) {
        std::cerr << "Invalid address: " << server->getAddress() << std::endl;
        closesocket(sock);
        return false;
    }

    // Attempt connection
    int result = connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "Connection failed for " << server->getId() 
                     << ", error: " << error << std::endl;
            closesocket(sock);
            return false;
        }

        // Use select to wait for connection
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);

        timeval tv;
        tv.tv_sec = 5;  // 5 seconds timeout
        tv.tv_usec = 0;

        result = select(0, nullptr, &write_fds, nullptr, &tv);
        if (result <= 0) {
            std::cerr << "Select failed or timed out for " << server->getId() 
                     << ", error: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            return false;
        }
    }

    // Set back to blocking mode
    mode = 0;
    ioctlsocket(sock, FIONBIO, &mode);

    closesocket(sock);
    return true;
}