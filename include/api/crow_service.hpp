#pragma once

#include "crow.h"
#include "nlohmann/json.hpp"
#include "core/server_manager.hpp"
#include <iostream>

void runCrowServer(std::shared_ptr<ServerManager> server_manager);