cmake_minimum_required(VERSION 3.15)
project(load_balancer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(gRPC CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)

# Generate protobuf and gRPC files
get_filename_component(lb_proto "proto/load_balancer.proto" ABSOLUTE)
get_filename_component(lb_proto_path "${lb_proto}" PATH)

# Generate protobuf files
set(lb_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/load_balancer.pb.cc")
set(lb_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/load_balancer.pb.h")
set(lb_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/load_balancer.grpc.pb.cc")
set(lb_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/load_balancer.grpc.pb.h")

add_custom_command(
    OUTPUT "${lb_proto_srcs}" "${lb_proto_hdrs}" "${lb_grpc_srcs}" "${lb_grpc_hdrs}"
    COMMAND protobuf::protoc
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         -I "${lb_proto_path}"
         --plugin=protoc-gen-grpc="$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
         "${lb_proto}"
    DEPENDS "${lb_proto}")

# Include generated files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(include)

# Create load balancer library
add_library(lb_lib
    src/core/server.cpp
    src/core/server_manager.cpp
    src/core/load_balancer.cpp
    src/strategies/round_robin.cpp
    ${lb_proto_srcs}
    ${lb_grpc_srcs})

target_link_libraries(lb_lib
    PUBLIC
    gRPC::grpc++
    protobuf::libprotobuf)

# Create main executable
add_executable(load_balancer src/main.cpp)
target_link_libraries(load_balancer PRIVATE lb_lib)

# Create backend server executable
add_executable(backend_server examples/backend_server.cpp)
target_link_libraries(backend_server PRIVATE lb_lib)