cmake_minimum_required(VERSION 3.15)
project(load_balancer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------
# 1) Find required packages: gRPC, Protobuf, Crow, and nlohmann_json
# -----------------------------------------------------------------------
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(Crow CONFIG REQUIRED)               # Provided by vcpkg install crow
find_package(nlohmann_json CONFIG REQUIRED)      # Provided by vcpkg install nlohmann-json

# -----------------------------------------------------------------------
# 2) List out all .cpp/.cc files:
#    - core LB
#    - strategies
#    - generated Protobuf/GRPC stubs
#    - admin service
#    - (Optionally) separate file for Crow code, if you prefer
# -----------------------------------------------------------------------
set(LIB_SOURCES
    src/core/server.cpp
    src/core/server_manager.cpp
    src/core/load_balancer.cpp
    src/core/process/process_factory.cpp
    src/core/process/windows_process.cpp

    src/strategies/round_robin.cpp
    src/strategies/least_connections.cpp
    #src/strategies/resource_based.cpp

    proto/load_balancer.pb.cc
    proto/load_balancer.grpc.pb.cc

    proto/admin_service.pb.cc
    proto/admin_service.grpc.pb.cc

    src/api/admin_service.cpp
    src/api/crow_service.cpp

    src/utils/config.cpp
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

# -----------------------------------------------------------------------
# 3) Create a library "lb_lib" that holds LB + admin code
# -----------------------------------------------------------------------
add_library(lb_lib ${LIB_SOURCES})
target_link_libraries(lb_lib
    PUBLIC
    gRPC::grpc++          # gRPC library
    protobuf::libprotobuf # Protobuf library
)

# -----------------------------------------------------------------------
# 4) Create the main load_balancer executable
#    - If your Crow server is integrated in main.cpp (or included .cpp),
#      you can link it here.
# -----------------------------------------------------------------------
add_executable(load_balancer
    src/main.cpp
    # If you have a separate file for Crow routes, add it here, e.g.:
    # src/admin/dashboard_api.cpp
)
target_link_libraries(load_balancer
    PRIVATE
    lb_lib
    Crow::Crow                             # Link Crow
    nlohmann_json::nlohmann_json           # Link nlohmann JSON
)

# -----------------------------------------------------------------------
# 5) Create the backend server executable
# -----------------------------------------------------------------------
add_executable(backend_server examples/backend_server.cpp)
target_link_libraries(backend_server
    PRIVATE
    lb_lib
)

# -----------------------------------------------------------------------
# 6) Create the health_checker executable
# -----------------------------------------------------------------------
add_executable(health_checker src/monitoring/health_checker.cpp)
target_link_libraries(health_checker
    PRIVATE
    lb_lib
)

# -----------------------------------------------------------------------
# 7) Set output directories for all executables
# -----------------------------------------------------------------------
set_target_properties(load_balancer backend_server health_checker
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
