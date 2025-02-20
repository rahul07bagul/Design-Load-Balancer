#pragma once
#include "strategies/strategy.hpp"
#include <atomic>
#include <memory>
#include <vector>

class RoundRobinStrategy : public Strategy {
public:
    std::shared_ptr<Server> selectServer(
        const std::vector<std::shared_ptr<Server>>& servers,
        const loadbalancer::Request& request) override;
    
private:
    std::atomic<size_t> current_index_{0};
};