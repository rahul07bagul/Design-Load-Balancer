# Load Balancer System

## Overview
This project implements a Load Balancer System for managing backend servers efficiently. It supports automatic server scaling, health checking, and request distribution using a Round-Robin, Least Connection, Resource Based Strategy. The system is built using C++ with gRPC for inter-service communication and Crow for HTTP-based management.

## Features
- Load Balancing: Distributes incoming requests among active backend servers.
- Server Management: Allows dynamic addition and removal of backend servers.
- Health Monitoring: Periodically checks server health and replaces unhealthy instances.
- Admin API: Provides gRPC-based server administration.
- HTTP API: Enables interaction with the system using RESTful endpoints.

## Architecture
![Design](https://github.com/rahul07bagul/load-balancer-cpp/blob/main/Load%20Balancer.png)

## Class Diagram
![Design](https://github.com/rahul07bagul/load-balancer-cpp/blob/main/Design.png)

## Installation & Setup
### Prerequisites
- C++17 or later
- gRPC and Protobuf
- Crow HTTP frameowrk
- CMake

### Sintallation
- Download vcpkg
```shell
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
```
- Install gRPC and protobuf
```shell
.\vcpkg install grpc:x64-windows
.\vcpkg install protobuf:x64-windows
.\vcpkg integrate install
```
### Running the Load Balancer
```shell
./load_balancer --backend-path ./server --port 50050 --min-servers 2 --max-servers 5 --start-port 50051
```
### Running the Health Checker
```shell
./health_checker 127.0.0.1:50050
```

## API Endpoints
### HTTP API (Crow)
- GET /api/status - Returns a list of active servers.
- POST /api/add_server - Adds a new backend server.
- POST /api/remove_server - Removes a server by ID.

## Contributing
Feel free to contribute by submitting pull requests or feature requests.
  




  
