#pragma once

#include "admin_service.grpc.pb.h"
#include "core/server_manager.hpp"
#include <memory>

class AdminService final : public admin::AdminService::Service {
public:
    explicit AdminService(std::shared_ptr<ServerManager> server_manager);

    // List all servers
    ::grpc::Status ListServers(::grpc::ServerContext* context,
                               const ::google::protobuf::Empty* request,
                               admin::ListServersResponse* response) override;

    // Update the health of a server
    ::grpc::Status UpdateServerHealth(::grpc::ServerContext* context,
                                      const admin::UpdateServerHealthRequests* request,
                                      ::google::protobuf::Empty* response) override;

    // Add a new server
    ::grpc::Status AddServer(::grpc::ServerContext* context,
                             const ::google::protobuf::Empty* request,
                             admin::AddServerResponse* response) override;

    // Remove a server
    ::grpc::Status RemoveServer(::grpc::ServerContext* context,
                                const admin::RemoveServerRequest* request,
                                ::google::protobuf::Empty* response) override;
    
    // Get server stats
    ::grpc::Status GetServerConstraints(::grpc::ServerContext* context,
                                  const ::google::protobuf::Empty* request,
                                  admin::ServerConstraintsResponse* response) override;
private:
    std::shared_ptr<ServerManager> server_manager_;
};
