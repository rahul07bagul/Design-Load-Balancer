#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <grpcpp/grpcpp.h>
#include <thread> 

#include "core/load_balancer.hpp"
#include "core/server_manager.hpp"
#include "api/admin_service.hpp"
#include "api/crow_service.hpp"
#include "utils/config.hpp"
#include "utils/helper.hpp"

std::unique_ptr<grpc::Server> g_server;
bool g_shutting_down = false;
std::shared_ptr<ServerManager> server_manager;

void signalHandler(int signum) {
    std::cout << "\nShutdown signal received. Cleaning up..." << std::endl;
    g_shutting_down = true;

    if (g_server) {
        std::cout << "Shutting down gRPC server..." << std::endl;
        g_server->Shutdown();
    }
}

int main(int argc, char** argv) {
    try {
        // Register signal handler
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // Parse command line arguments
        Config config = parseArgs(argc, argv);
        std::shared_ptr<Configuration> configuration = Configuration::getInstance();
        
        std::cout << "Initializing load balancer with configuration:\n"
                  << "  Backend path: " << config.backend_path << "\n"
                  << "  Load balancer port: " << config.lb_port << "\n"
                  << "  Start port: " << config.start_port << "\n"
                  << "  Min servers: " << config.min_servers << "\n"
                  << "  Max servers: " << config.max_servers << std::endl;
        
        // server manager
        server_manager = std::make_shared<ServerManager>(
            config.backend_path,
            config.start_port,
            config.min_servers,
            config.max_servers
        );
        
        // load balancing strategy
        auto strategy = configuration->getStrategy();
        
        // load balancer service
        LoadBalancerService service(server_manager, strategy);

        auto admin_service = std::make_unique<AdminService>(server_manager);

        std::thread crowThread(runCrowServer, server_manager);
        
        // Setup and start gRPC server
        std::string server_address = std::string(load_balancer_address) +":"+ std::to_string(config.lb_port);
        grpc::ServerBuilder builder;
        
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
        builder.RegisterService(admin_service.get());
        
        g_server = builder.BuildAndStart();
        std::cout << "Load Balancer started at: " << server_address << std::endl;
        
        g_server->Wait();

        std::cout << "Cleanup complete. Exiting." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}