#include <iostream>
#include <memory>
#include <string>
#include <grpcpp/grpcpp.h>
#include "core/load_balancer.hpp"
#include "core/server_manager.hpp"
#include "strategies/round_robin.hpp"

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  --backend-path PATH   Path to backend server executable (required)\n"
              << "  --port PORT           Load balancer port (default: 50050)\n"
              << "  --min-servers N       Minimum number of backend servers (default: 2)\n"
              << "  --max-servers N       Maximum number of backend servers (default: 5)\n"
              << "  --start-port N        Starting port for backend servers (default: 50051)\n";
}

struct Config {
    std::string backend_path;
    uint16_t lb_port = 50050;
    uint16_t start_port = 50051;
    size_t min_servers = 2;
    size_t max_servers = 5;
};

Config parseArgs(int argc, char** argv) {
    Config config;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--backend-path" && i + 1 < argc) {
            config.backend_path = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            config.lb_port = std::stoi(argv[++i]);
        } else if (arg == "--min-servers" && i + 1 < argc) {
            config.min_servers = std::stoi(argv[++i]);
        } else if (arg == "--max-servers" && i + 1 < argc) {
            config.max_servers = std::stoi(argv[++i]);
        } else if (arg == "--start-port" && i + 1 < argc) {
            config.start_port = std::stoi(argv[++i]);
        } else {
            printUsage(argv[0]);
            exit(1);
        }
    }
    
    if (config.backend_path.empty()) {
        std::cerr << "Error: --backend-path is required\n";
        printUsage(argv[0]);
        exit(1);
    }
    
    return config;
}

int main(int argc, char** argv) {
    try {
        // Parse command line arguments
        Config config = parseArgs(argc, argv);
        
        // Create server manager
        auto server_manager = std::make_shared<ServerManager>(
            config.backend_path,
            config.start_port,
            config.min_servers,
            config.max_servers
        );
        
        // Create load balancing strategy
        auto strategy = std::make_shared<RoundRobinStrategy>();
        
        // Create load balancer service
        LoadBalancerService service(server_manager, strategy);
        
        // Setup and start gRPC server
        std::string server_address = std::string("0.0.0.0:") + std::to_string(config.lb_port);
        grpc::ServerBuilder builder;
        
        // Add listening port to the server
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        
        // Register service
        builder.RegisterService(&service);
        
        // Start the server
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "Load Balancer started at: " << server_address << std::endl;
        std::cout << "Backend servers configuration:" << std::endl;
        std::cout << "  - Number of servers: " << config.min_servers << " to " 
                 << config.max_servers << std::endl;
        std::cout << "  - Starting port: " << config.start_port << std::endl;
        
        // Keep the server running
        server->Wait();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}