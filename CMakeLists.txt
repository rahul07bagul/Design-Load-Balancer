# CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
project(load_balancer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Source files for the library
set(LIB_SOURCES
    src/core/server.cpp
    src/core/server_manager.cpp
    src/core/load_balancer.cpp
    src/strategies/round_robin.cpp
    proto/load_balancer.pb.cc
    proto/load_balancer.grpc.pb.cc
)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/proto
)

# Create load balancer library
add_library(lb_lib ${LIB_SOURCES})
target_link_libraries(lb_lib
    PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf
)

# Create main executable
add_executable(load_balancer src/main.cpp)
target_link_libraries(load_balancer
    PRIVATE
    lb_lib
)

# Create backend server executable
add_executable(backend_server examples/backend_server.cpp)
target_link_libraries(backend_server
    PRIVATE
    lb_lib
)

# Set output directories
set_target_properties(load_balancer backend_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)