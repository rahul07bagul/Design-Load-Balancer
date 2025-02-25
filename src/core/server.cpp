#include "core/server.hpp"

Server::Server(const std::string& host, int port)
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

int Server::getPort() const {
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

double Server::getCPUUsage() const {
    return cpu_usage;
}

void Server::setCPUUsage(double usage) {
    cpu_usage = usage;
}

double Server::getMemoryUsage() const {
    return memory_usage;
}

void Server::setMemoryUsage(double usage) {
    memory_usage = usage;
}

void Server::setLastHealthCheckTime(std::chrono::system_clock::time_point t) {
    last_health_check_time_ = t;
}

std::chrono::system_clock::time_point Server::getLastHealthCheckTime() const {
    return last_health_check_time_;
}

void Server::incrementActiveConnections() {
    active_connections_.fetch_add(1, std::memory_order_relaxed);
}

void Server::decrementActiveConnections() {
    active_connections_.fetch_sub(1, std::memory_order_relaxed);
}

int Server::getActiveConnections() const {
    return active_connections_.load(std::memory_order_relaxed);
}