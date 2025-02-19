#pragma once
#include <string>
#include <cstdint>

class Server {
public:
    Server(const std::string& host, uint16_t port);
    std::string getAddress() const;
    uint16_t getPort() const;
    std::string getId() const;
    bool isHealthy() const;
    void setHealthStatus(bool status);

private:
    std::string host_;
    uint16_t port_;
    bool is_healthy_;
    std::string id_;
};