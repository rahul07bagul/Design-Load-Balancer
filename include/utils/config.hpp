#pragma once
#include <string>
#include "strategies/round_robin.hpp"

// Declare as extern to indicate they're defined elsewhere
extern std::string server_address;
extern std::string load_balancer_address;
extern int health_checker_sleep_time;

struct Config {
    std::string backend_path;
    int lb_port = 50050;
    int start_port = 50051;
    size_t min_servers = 2;
    size_t max_servers = 5;
};

class Configuration {
private:
    Configuration() {};
    static std::shared_ptr<Configuration> instance;
public:
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    static std::shared_ptr<Configuration> getInstance() {
        if (instance == nullptr) {
            instance = std::shared_ptr<Configuration>(new Configuration());
        }
        return instance;
    }

    std::shared_ptr<Strategy> getStrategy() {
        return std::make_shared<RoundRobinStrategy>();
    };
};