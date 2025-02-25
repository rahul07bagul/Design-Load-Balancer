#include "utils/config.hpp"
#include <iostream>

void printUsage(const char* programName) {
    std::cerr << "Usage: " << programName << " [options]\n"
              << "Options:\n"
              << "  --backend-path PATH   Path to backend server executable (required)\n"
              << "  --port PORT           Load balancer port (default: 50050)\n"
              << "  --min-servers N       Minimum number of backend servers (default: 2)\n"
              << "  --max-servers N       Maximum number of backend servers (default: 5)\n"
              << "  --start-port N        Starting port for backend servers (default: 50051)\n";
}

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
                config.lb_port = static_cast<int>(std::stoi(argv[++i]));
            } else if (arg == "--min-servers") {
                config.min_servers = static_cast<size_t>(std::stoi(argv[++i]));
            } else if (arg == "--max-servers") {
                config.max_servers = static_cast<size_t>(std::stoi(argv[++i]));
            } else if (arg == "--start-port") {
                config.start_port = static_cast<int>(std::stoi(argv[++i]));
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