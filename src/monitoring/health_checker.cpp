#include <iostream>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "proto/admin_service.grpc.pb.h"
#include "monitoring/health_checker.hpp"

using namespace std::chrono_literals;

static bool running = true; // for stopping gracefully

// A function that retrieves the server list from the AdminService
std::vector<admin::ServerInfo> listAllServers(std::unique_ptr<admin::AdminService::Stub>& stub) {
    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    admin::ListServersResponse response;

    auto status = stub->ListServers(&ctx, empty, &response);
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

bool isServerResponding(const std::string& host, uint16_t port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set socket timeout
    DWORD timeout = 5000; // 5 seconds
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
    server_addr.sin_port = htons(port);

    // Convert address with error checking
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

// A function to update a server's health via AdminService
void updateServerHealth(std::unique_ptr<admin::AdminService::Stub>& stub,
                        const std::string& serverId,
                        bool isHealthy) {
    admin::UpdateServerHealthRequest req;
    req.set_id(serverId);
    req.set_ishealthy(isHealthy);

    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    auto status = stub->UpdateServerHealth(&ctx, req, &empty);
    if (!status.ok()) {
        std::cerr << "UpdateServerHealth RPC failed for " << serverId
                  << ": " << status.error_message() << std::endl;
    }
}

// A function to add a new server via AdminService
void addServer(std::unique_ptr<admin::AdminService::Stub>& stub) {
    grpc::ClientContext ctx;
    google::protobuf::Empty empty;
    admin::AddServerResponse resp;
    auto status = stub->AddServer(&ctx, empty, &resp);
    if (!status.ok()) {
        std::cerr << "AddServer RPC failed: " << status.error_message() << std::endl;
    } else {
        std::cout << "Created new server with ID " << resp.id() << std::endl;
    }
}

int main(int argc, char** argv) {
    // Parse command line args to get LB admin address, e.g. "127.0.0.1:50050"
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <load_balancer_admin_address>\n"
                  << " e.g.: health_checker_main 127.0.0.1:50050\n";
        return 1;
    }
    std::string lb_admin_address = argv[1];

    // 1) Create a channel and stub to talk to the AdminService
    auto channel = grpc::CreateChannel(lb_admin_address, grpc::InsecureChannelCredentials());
    auto stub = admin::AdminService::NewStub(channel);

    while (running) {
        std::cout << "\n=== External Health Check Started ===" << std::endl;

        // 2) Get the full list of servers
        auto servers = listAllServers(stub);

        // 3) For each server, do the same 'isServerResponding' logic you already have.
        for (auto& s : servers) {
            bool currentHealth = s.ishealthy();
            bool check = isServerResponding(s.host(), s.port()); // We'll define this next
            
            std::cout << "Checking server " << s.id() << ":\n"
                      << "  Status: " << (check ? "Healthy" : "Unhealthy") << std::endl;

            if (currentHealth && !check) {
                // Mark as down
                std::cout << "Server " << s.id() << " is down" << std::endl;
                updateServerHealth(stub, s.id(), false);

                // Possibly add a new server
                addServer(stub);
            } else if (!currentHealth && check) {
                // Mark as healthy again
                std::cout << "Server " << s.id() << " is back online" << std::endl;
                updateServerHealth(stub, s.id(), true);
            }
        }

        std::cout << "=== Health Check Completed ===" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
}
