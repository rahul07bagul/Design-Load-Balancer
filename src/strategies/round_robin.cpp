#include "strategies/round_robin.hpp"

std::shared_ptr<Server> RoundRobinStrategy::selectServer(
    const std::vector<std::shared_ptr<Server>>& servers,
    const loadbalancer::Request& request) {
    
    if (servers.empty()) {
        return nullptr;
    }

    size_t index = current_index_++;
    if (current_index_ >= servers.size()) {
        current_index_ = 0;
    }

    return servers[index % servers.size()];
}