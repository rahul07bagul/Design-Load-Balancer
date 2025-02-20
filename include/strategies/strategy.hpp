#pragma once
#include <memory>
#include <vector>
#include "proto/load_balancer.pb.h"
#include "core/server.hpp"

class Strategy {
public:
    virtual ~Strategy() = default;
    virtual std::shared_ptr<Server> selectServer(
        const std::vector<std::shared_ptr<Server>>& servers,
        const loadbalancer::Request& request) = 0;
};