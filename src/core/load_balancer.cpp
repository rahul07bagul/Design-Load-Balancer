#include "core/load_balancer.hpp"
#include <grpcpp/grpcpp.h>

LoadBalancerService::LoadBalancerService(
    std::shared_ptr<ServerManager> server_manager,
    std::shared_ptr<Strategy> strategy)
    : server_manager_(server_manager)
    , strategy_(strategy) {}

grpc::Status LoadBalancerService::HandleRequest(
    grpc::ServerContext* context,
    const loadbalancer::Request* request,
    loadbalancer::Response* response) {

    auto servers = server_manager_->getActiveServers();
    auto selected_server = strategy_->selectServer(servers, *request);

    if (!selected_server) {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "No servers available");
    }

    // Create client and forward request to selected server
    std::string server_address = selected_server->getAddress() + ":" + 
                                std::to_string(selected_server->getPort());
    
    auto channel = grpc::CreateChannel(
        server_address, grpc::InsecureChannelCredentials());
    auto stub = loadbalancer::LoadBalancerService::NewStub(channel);

    grpc::ClientContext client_context;
    loadbalancer::Response server_response;

    auto status = stub->HandleRequest(&client_context, *request, &server_response);
    
    if (status.ok()) {
        response->set_message(server_response.message());
        response->set_server_id(selected_server->getId());
        return grpc::Status::OK;
    }

    return status;
}