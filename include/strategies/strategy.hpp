#pragma once
#include <memory>
#include <vector>
#include "core/server.hpp"
#include "load_balancer.grpc.pb.h"

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual std::shared_ptr<Server> selectServer(
        const std::vector<std::shared_ptr<Server>>& servers,
        const loadbalancer::Request& request) = 0;
};

// include/strategies/round_robin.hpp
#pragma once
#include "strategies/strategy.hpp"
#include <atomic>

class RoundRobinStrategy : public Strategy {
public:
    std::shared_ptr<Server> selectServer(
        const std::vector<std::shared_ptr<Server>>& servers,
        const loadbalancer::Request& request) override;
    
private:
    std::atomic<size_t> current_index_{0};
};