#pragma once
#include <string>
#include <cstdint>
#include <windows.h>

class Server {
public:
    Server(const std::string& host, uint16_t port);
    std::string getAddress() const;
    uint16_t getPort() const;
    std::string getId() const;
    bool isHealthy() const;
    void setHealthStatus(bool status);
    DWORD getProcessId() const { return process_id_; }
    void setProcessId(DWORD pid) { process_id_ = pid; }

private:
    std::string host_;
    uint16_t port_;
    bool is_healthy_;
    std::string id_;
    DWORD process_id_;
};