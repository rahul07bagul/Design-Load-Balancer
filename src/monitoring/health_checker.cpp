#include <iostream>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "proto/admin_service.grpc.pb.h"
#include "monitoring/health_checker.hpp"
#include "utils/config.hpp"

using namespace std::chrono_literals;

const double SCALE_UP_CPU_THRESHOLD = 80.0;
const double SCALE_DOWN_CPU_THRESHOLD = 20.0;
static const double NEW_SERVER_USAGE = 0.0;

HealthChecker::HealthChecker(const std::string& lb_admin_address)
    : running_(false) {
    auto channel = grpc::CreateChannel(lb_admin_address, grpc::InsecureChannelCredentials());
    admin_stub_ = admin::AdminService::NewStub(channel);
}

HealthChecker::~HealthChecker() {
    stop();
}

void HealthChecker::start() {
    if (!running_) {
        running_ = true;
        health_check_thread_ = std::make_unique<std::thread>(&HealthChecker::checkHealth, this);
    }
}

void HealthChecker::stop() {
    if (running_) {
        running_ = false;
        if (health_check_thread_ && health_check_thread_->joinable()) {
            health_check_thread_->join();
        }
    }
}

std::vector<admin::ServerInfo> HealthChecker::listAllServers() {
    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    admin::ListServersResponse response;

    auto status = admin_stub_->ListServers(&ctx, empty, &response);
    if (!status.ok()) {
        std::cerr << "ListServers RPC failed: " << status.error_message() << std::endl;
        return {};
    }

    std::vector<admin::ServerInfo> result;
    for (auto& server : *response.mutable_servers()) {
        result.push_back(std::move(server));
    }
    return result;
}

bool HealthChecker::isServerResponding(const std::string& host, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set socket timeout
    DWORD timeout = 5000; // 5 seconds
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        std::cerr << "Failed to set non-blocking mode: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        return false;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) != 1) {
        std::cerr << "Invalid address: " << host << std::endl;
        closesocket(sock);
        return false;
    }

    // Attempt connection
    int result = connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
    if (result == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "Connection failed for " << host << ":" << port
                      << ", error: " << error << std::endl;
            closesocket(sock);
            return false;
        }

        // Use select to wait for the connection
        fd_set write_fds;
        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);

        timeval tv;
        tv.tv_sec = 5;  // 5 seconds
        tv.tv_usec = 0;

        result = select(0, nullptr, &write_fds, nullptr, &tv);
        if (result <= 0) {
            std::cerr << "Select failed or timed out for " << host << ":" << port
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

void HealthChecker::updateServerHealth(const std::vector<admin::UpdateServerHealthRequest>& updates) {
    admin::UpdateServerHealthRequests req;
    
    for (auto& u : updates) {
        auto* s = req.add_updates();
        s->set_id(u.id());
        s->set_ishealthy(u.ishealthy());
        s->set_cpu_usage(u.cpu_usage());
        s->set_memory_usage(u.memory_usage());
    }

    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    auto status = admin_stub_->UpdateServerHealth(&ctx, req, &empty);
    
    if (!status.ok()) {
        std::cerr << "UpdateServerHealth RPC failed!\n"
                  << "Status code: " << status.error_code() << "\n"
                  << "Message: " << status.error_message() << "\n"
                  << "Details: " << status.error_details() << std::endl;
    } else {
        std::cout << "Health update completed successfully" << std::endl;
    }
}

bool HealthChecker::getServerMetrics(const std::string& host, int port, double& outCpu, double& outMem) {
    std::string target = host + ":" + std::to_string(port);
    auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    auto stub = admin::AdminService::NewStub(channel);

    google::protobuf::Empty empty;
    admin::MetricsResponse resp;
    grpc::ClientContext ctx;
    auto status = stub->GetMetrics(&ctx, empty, &resp);
    if (!status.ok()) {
        std::cerr << "GetMetrics RPC failed for " << target 
                  << ": " << status.error_message() << std::endl;
        return false;
    }
    outCpu = resp.cpu_usage();
    outMem = resp.memory_usage();
    return true;
}

admin::ServerConstraintsResponse HealthChecker::getServerLimits() {
    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    admin::ServerConstraintsResponse resp;
    auto status = admin_stub_->GetServerConstraints(&ctx, empty, &resp);
    if (!status.ok()) {
        std::cerr << "GetServerConstraints RPC failed: " << status.error_message() << std::endl;
    }
    return resp;
}

void HealthChecker::addServer() {
    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    admin::AddServerResponse resp;
    auto status = admin_stub_->AddServer(&ctx, empty, &resp);
    if (!status.ok()) {
        std::cerr << "AddServer RPC failed: " << status.error_message() << std::endl;
    } else {
        std::cout << "Created new server with ID " << resp.id() << std::endl;
    }
}

void HealthChecker::removeServer(const std::string& serverId) {
    admin::RemoveServerRequest req;
    req.set_id(serverId);

    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    auto status = admin_stub_->RemoveServer(&ctx, req, &empty);
    if (!status.ok()) {
        std::cerr << "RemoveServer RPC failed for " << serverId
                  << ": " << status.error_message() << std::endl;
    }
}

void HealthChecker::handleAutoScaling(double cpu, const std::string& serverId, 
                                     const admin::ServerConstraintsResponse& constraints) {
    if (cpu > SCALE_UP_CPU_THRESHOLD && constraints.active_servers() < constraints.max_servers()) {
        std::cout << "Scaling up due to high CPU on server " << serverId << std::endl;
        //Scale Up
        addServer();
    } else if (cpu < SCALE_DOWN_CPU_THRESHOLD && constraints.active_servers() > constraints.min_servers()) {
        std::cout << "Scaling down server " << serverId << " due to low CPU usage" << std::endl;
        //Scale Down
        removeServer(serverId);
    } else {
        // No scaling needed or possible
        if (cpu > SCALE_UP_CPU_THRESHOLD) {
            std::cout << "Server " << serverId << " CPU high but already at max servers limit" << std::endl;
        } else if (cpu < SCALE_DOWN_CPU_THRESHOLD) {
            std::cout << "Server " << serverId << " CPU low but already at min servers limit" << std::endl;
        }
    }
}

void HealthChecker::checkServersOnce() {
    std::cout << "\n=== External Health Check Started ===" << std::endl;

    auto servers = listAllServers();
    auto constraints = getServerLimits();
    std::cout << "Active servers: " << constraints.active_servers() << std::endl;
    std::vector<admin::UpdateServerHealthRequest> updates;  

    for (auto& s : servers) {
        admin::UpdateServerHealthRequest server_metrics;
        server_metrics.set_id(s.id());

        bool currentHealth = s.ishealthy();
        bool check = isServerResponding(s.host(), s.port());
        
        std::cout << "Checking server " << s.id() << ":\n"
                  << "  Status: " << (check ? "Healthy" : "Unhealthy") << std::endl;

        if (currentHealth && !check) {
            std::cout << "Server " << s.id() << " is down" << std::endl;
            //Current server is unhealthy
            server_metrics.set_ishealthy(false);
            //Add new server
            addServer();
        } else if(check){
            server_metrics.set_ishealthy(true);

            double cpu, mem;
            if (getServerMetrics(s.host(), s.port(), cpu, mem)) {
                server_metrics.set_cpu_usage(cpu);
                server_metrics.set_memory_usage(mem);

                std::cout << "Server " << s.id() << ":\n"
                          << "  CPU: " << cpu << "%\n"
                          << "  Memory: " << mem << "%\n";
                
                handleAutoScaling(cpu, s.id(), constraints);
            }
        }
        updates.push_back(server_metrics);
    }

    updateServerHealth(updates);

    std::cout << "=== Health Check Completed ===" << std::endl;
}

void HealthChecker::checkHealth() {
    while (running_) {
        checkServersOnce();
        std::this_thread::sleep_for(std::chrono::seconds(health_checker_sleep_time));
    }
}


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <load_balancer_admin_address>\n"
                  << " e.g.: health_checker_main 127.0.0.1:50050\n";
        return 1;
    }
    std::string lb_admin_address = argv[1];
    
    HealthChecker healthChecker(lb_admin_address);
    
    // healthChecker.checkServersOnce();
    
    // Run continuous health checks in a dedicated thread
    healthChecker.start();
    
    std::cout << "Health checker running. Press Ctrl+C to stop." << std::endl;
    
    std::string input;
    std::getline(std::cin, input);
    
    healthChecker.stop();

    return 0;
}
