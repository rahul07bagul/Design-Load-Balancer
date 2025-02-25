#include "utils/config.hpp"

// Define the global variables
std::string server_address = "127.0.0.1";
std::string load_balancer_address = "0.0.0.0";

// Define the static member
std::shared_ptr<Configuration> Configuration::instance = nullptr;