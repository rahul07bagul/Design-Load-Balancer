#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <grpcpp/grpcpp.h>
#include "proto/admin_service.grpc.pb.h"

#pragma comment(lib, "Ws2_32.lib")

// Constants
extern const double SCALE_UP_CPU_THRESHOLD;
extern const double SCALE_DOWN_CPU_THRESHOLD;
extern int health_checker_sleep_time;

class HealthChecker {
public:
    explicit HealthChecker(const std::string& lb_admin_address);
    ~HealthChecker();

    // Non-copyable
    HealthChecker(const HealthChecker&) = delete;
    HealthChecker& operator=(const HealthChecker&) = delete;

    void start();
    void stop();
    void checkServersOnce(); 

private:
    void checkHealth();
    
    std::vector<admin::ServerInfo> listAllServers();
    bool isServerResponding(const std::string& host, int port);
    void updateServerHealth(const std::vector<admin::UpdateServerHealthRequest>& updates);
    bool getServerMetrics(const std::string& host, int port, double& outCpu, double& outMem);
    admin::ServerConstraintsResponse getServerLimits();
    void addServer();
    void removeServer(const std::string& serverId);
    void handleAutoScaling(double cpu, const std::string& serverId, const admin::ServerConstraintsResponse& constraints);

    std::atomic<bool> running_;
    std::unique_ptr<std::thread> health_check_thread_;
    std::unique_ptr<admin::AdminService::Stub> admin_stub_;
};