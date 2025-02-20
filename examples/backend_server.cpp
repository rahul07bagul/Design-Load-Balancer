#include <iostream>
#include <string>
#include <grpcpp/grpcpp.h>
#include "proto/load_balancer.grpc.pb.h"

class BackendServer final : public loadbalancer::LoadBalancerService::Service {
public:
    grpc::Status HandleRequest(grpc::ServerContext* context,
                             const loadbalancer::Request* request,
                             loadbalancer::Response* response) override {
        std::cout << "Backend server received request: " << request->message() 
                 << " on port: " << port_ << std::endl;
        
        response->set_message("Response from backend server on port: " + 
                            std::to_string(port_));
        response->set_server_id("backend_" + std::to_string(port_));
        
        return grpc::Status::OK;
    }

    void setPort(int port) { port_ = port; }

private:
    int port_;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    const int port = std::stoi(argv[1]);
    const std::string server_address = "0.0.0.0:" + std::to_string(port);

    BackendServer service;
    service.setPort(port);

    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Backend server listening on port: " << port << std::endl;

    server->Wait();

    return 0;
}