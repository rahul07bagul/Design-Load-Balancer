cmake_minimum_required(VERSION 3.15)
project(load_balancer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# ------------------------------------------------------------------------------
# 1) List out all .cpp/.cc files, including both load_balancer and admin_service
#    generated code. Make sure you have admin_service.proto compiled to
#    admin_service.pb.cc/.h and admin_service.grpc.pb.cc/.h
# ------------------------------------------------------------------------------
set(LIB_SOURCES
    # Core LB sources
    src/core/server.cpp
    src/core/server_manager.cpp
    src/core/load_balancer.cpp
    src/strategies/round_robin.cpp

    # Generated gRPC/protobuf sources for load_balancer.proto
    proto/load_balancer.pb.cc
    proto/load_balancer.grpc.pb.cc

    # Generated gRPC/protobuf sources for admin_service.proto
    proto/admin_service.pb.cc
    proto/admin_service.grpc.pb.cc

    src/admin/admin_service.cpp
)

# ------------------------------------------------------------------------------
# 2) Include directories
#    - The "proto" folder for generated headers
#    - The "include" folder for your own .hpp files
# ------------------------------------------------------------------------------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

# ------------------------------------------------------------------------------
# 3) Create a library "lb_lib" that holds our load balancer + admin code
# ------------------------------------------------------------------------------
add_library(lb_lib ${LIB_SOURCES})
target_link_libraries(lb_lib
    PUBLIC
    gRPC::grpc++          # gRPC library
    protobuf::libprotobuf # Protobuf library
)

# ------------------------------------------------------------------------------
# 4) Create the main load_balancer executable
# ------------------------------------------------------------------------------
add_executable(load_balancer src/main.cpp)
target_link_libraries(load_balancer
    PRIVATE
    lb_lib
)

# ------------------------------------------------------------------------------
# 5) Create the backend server executable
# ------------------------------------------------------------------------------
add_executable(backend_server examples/backend_server.cpp)
target_link_libraries(backend_server
    PRIVATE
    lb_lib
)

# ------------------------------------------------------------------------------
# 6) Create the new health_checker executable
#    (assuming your code is in src/monitoring/health_checker_main.cpp)
# ------------------------------------------------------------------------------
add_executable(health_checker src/monitoring/health_checker.cpp)
target_link_libraries(health_checker
    PRIVATE
    lb_lib
)

# ------------------------------------------------------------------------------
# 7) Set the output directories for all executables
# ------------------------------------------------------------------------------
set_target_properties(load_balancer backend_server health_checker
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
