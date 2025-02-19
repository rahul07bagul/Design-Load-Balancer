#include "core/server.hpp"

Server::Server(const std::string& host, uint16_t port)
    : host_(host), port_(port), is_healthy_(true) {
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
}