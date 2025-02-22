#include "core/server.hpp"

Server::Server(const std::string& host, uint16_t port)
    : host_(host)
    , port_(port)
    , is_healthy_(true)
    , last_health_check_time_(std::chrono::system_clock::now())
{
    id_ = host + ":" + std::to_string(port);
}

std::string Server::getAddress() const {
    return host_;
}

uint16_t Server::getPort() const {
    return port_;
}

std::string Server::getId() const {
    return id_;
}

bool Server::isHealthy() const {
    return is_healthy_;
}

void Server::setHealthStatus(bool status) {
    is_healthy_ = status;
    last_health_check_time_ = std::chrono::system_clock::now();
}

void Server::setLastHealthCheckTime(std::chrono::system_clock::time_point t) {
    last_health_check_time_ = t;
}

std::chrono::system_clock::time_point Server::getLastHealthCheckTime() const {
    return last_health_check_time_;
}