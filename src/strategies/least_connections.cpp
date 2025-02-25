#include "strategies/least_connections.hpp"

std::shared_ptr<Server> LeastConnectionsStrategy::selectServer(
    const std::vector<std::shared_ptr<Server>>& servers,
    const loadbalancer::Request& request) {
    std::shared_ptr<Server> best_server = nullptr;
    int min_connections = INT_MAX;

    for (const auto& server : servers) {
        if (!server->isHealthy()) continue;

        int active_connections = server->getActiveConnections();
        if (active_connections < min_connections) {
            min_connections = active_connections;
            best_server = server;
        }
    }

    return best_server;
}