#include <iostream>
#include <memory>
#include <string>
#include <csignal>
#include <grpcpp/grpcpp.h>
#include "core/load_balancer.hpp"
#include "core/server_manager.hpp"
#include "strategies/round_robin.hpp"
#include "admin/admin_service.hpp"

std::unique_ptr<grpc::Server> g_server;
bool g_shutting_down = false;

void signalHandler(int signum) {
    std::cout << "\nShutdown signal received. Cleaning up..." << std::endl;
    g_shutting_down = true;

    if (g_server) {
        std::cout << "Shutting down gRPC server..." << std::endl;
        g_server->Shutdown();
    }
}

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
        
        if (i + 1 >= argc && arg != "--help" && arg != "-h") {
            std::cerr << "Error: Missing value for argument " << arg << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
        
        try {
            if (arg == "--backend-path") {
                config.backend_path = argv[++i];
            } else if (arg == "--port") {
                config.lb_port = static_cast<uint16_t>(std::stoi(argv[++i]));
            } else if (arg == "--min-servers") {
                config.min_servers = static_cast<size_t>(std::stoi(argv[++i]));
            } else if (arg == "--max-servers") {
                config.max_servers = static_cast<size_t>(std::stoi(argv[++i]));
            } else if (arg == "--start-port") {
                config.start_port = static_cast<uint16_t>(std::stoi(argv[++i]));
            } else if (arg == "--help" || arg == "-h") {
                printUsage(argv[0]);
                exit(0);
            } else {
                std::cerr << "Unknown argument: " << arg << std::endl;
                printUsage(argv[0]);
                exit(1);
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing argument " << arg << ": " << e.what() << std::endl;
            printUsage(argv[0]);
            exit(1);
        }
    }
    
    return config;
}

int main(int argc, char** argv) {
    try {
        // Register signal handler
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);

        // Parse command line arguments
        Config config = parseArgs(argc, argv);
        
        std::cout << "Initializing load balancer with configuration:\n"
                  << "  Backend path: " << config.backend_path << "\n"
                  << "  Load balancer port: " << config.lb_port << "\n"
                  << "  Start port: " << config.start_port << "\n"
                  << "  Min servers: " << config.min_servers << "\n"
                  << "  Max servers: " << config.max_servers << std::endl;
        
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

        auto admin_service = std::make_unique<AdminService>(server_manager);
        
        // Setup and start gRPC server
        std::string server_address = std::string("0.0.0.0:") + std::to_string(config.lb_port);
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