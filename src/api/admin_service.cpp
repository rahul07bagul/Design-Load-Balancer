#include "api/admin_service.hpp"

AdminService::AdminService(std::shared_ptr<ServerManager> server_manager)
    : server_manager_(std::move(server_manager)) {}

::grpc::Status AdminService::ListServers(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, admin::ListServersResponse* response) {
    auto servers = server_manager_->getActiveServers(); 

    for (const auto& server : servers) {
        admin::ServerInfo* info = response->add_servers();
        info->set_id(server->getId());
        info->set_host(server->getAddress());
        info->set_port(server->getPort());
        info->set_ishealthy(server->isHealthy());
        auto tp = server->getLastHealthCheckTime();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            tp.time_since_epoch());
        info->set_last_health_check_unix_seconds(duration.count());
        info->set_request_count(server->getRequestCount());
    }

    return ::grpc::Status::OK;
}

::grpc::Status AdminService::UpdateServerHealth(::grpc::ServerContext* context, const admin::UpdateServerHealthRequests* request, ::google::protobuf::Empty* response) {
    for (const auto& update : request->updates()) {
        server_manager_->updateServerHealth(
            update.id(),
            update.ishealthy(),
            update.cpu_usage(),
            update.memory_usage()
        );
    }
    return ::grpc::Status::OK;
}

::grpc::Status AdminService::AddServer( ::grpc::ServerContext* context, const ::google::protobuf::Empty* request, admin::AddServerResponse* response) {
    auto latestServer = server_manager_->addServer();
    if (!latestServer) {
        return ::grpc::Status(::grpc::StatusCode::RESOURCE_EXHAUSTED,
                              "Max servers reached or cannot add");
    }
    response->set_id(latestServer->getId());
    return ::grpc::Status::OK;
}

::grpc::Status AdminService::RemoveServer(::grpc::ServerContext* context, const admin::RemoveServerRequest* request, ::google::protobuf::Empty* response) {
    bool success = server_manager_->removeServerById(request->id());
    if (!success) {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              "Could not remove server (not found or min servers reached)");
    }
    return ::grpc::Status::OK;
}

::grpc::Status AdminService::GetServerConstraints(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, admin::ServerConstraintsResponse* response) {
    auto sc = server_manager_->getServerStats();
    response->set_min_servers(static_cast<int>(sc.min_servers));
    response->set_max_servers(static_cast<int>(sc.max_servers));
    response->set_active_servers(static_cast<int>(sc.active_servers));
    return ::grpc::Status::OK;
}
