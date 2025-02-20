#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "proto/load_balancer.grpc.pb.h"

class Client {
public:
    Client(const std::string& address) {
        channel_ = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
        stub_ = loadbalancer::LoadBalancerService::NewStub(channel_);
    }

    void SendRequest(const std::string& message) {
        loadbalancer::Request request;
        request.set_message(message);

        loadbalancer::Response response;
        grpc::ClientContext context;

        grpc::Status status = stub_->HandleRequest(&context, request, &response);

        if (status.ok()) {
            std::cout << "Response received: " << response.message() << std::endl;
            std::cout << "Handled by server: " << response.server_id() << std::endl;
        } else {
            std::cout << "RPC failed: " << status.error_message() << std::endl;
        }
    }

private:
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<loadbalancer::LoadBalancerService::Stub> stub_;
};

int main() {
    // Connect to the load balancer
    Client client("localhost:50050");  // Load balancer address

    // Send multiple requests to see round-robin in action
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\nSending request " << i << std::endl;
        client.SendRequest("Test request " + std::to_string(i));
        
        // Small delay to make output more readable
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}