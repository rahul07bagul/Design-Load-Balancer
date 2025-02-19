#pragma once
#include <memory>
#include <grpcpp/grpcpp.h>
#include "proto/load_balancer.grpc.pb.h"
#include "core/server_manager.hpp"
#include "strategies/strategy.hpp"

class LoadBalancerService final : public loadbalancer::LoadBalancerService::Service {
public:
    LoadBalancerService(std::shared_ptr<ServerManager> server_manager,
                       std::shared_ptr<Strategy> strategy);

    grpc::Status HandleRequest(
        grpc::ServerContext* context,
        const loadbalancer::Request* request,
        loadbalancer::Response* response) override;

private:
    std::shared_ptr<ServerManager> server_manager_;
    std::shared_ptr<Strategy> strategy_;
};