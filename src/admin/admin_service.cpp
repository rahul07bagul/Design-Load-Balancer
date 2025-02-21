#include "admin/admin_service.hpp"

AdminService::AdminService(std::shared_ptr<ServerManager> server_manager)
    : server_manager_(std::move(server_manager)) {}

::grpc::Status AdminService::ListServers(
    ::grpc::ServerContext* context,
    const ::google::protobuf::Empty* request,
    admin::ListServersResponse* response)
{
    auto servers = server_manager_->getAllServers(); 

    for (const auto& server : servers) {
        admin::ServerInfo* info = response->add_servers();
        info->set_id(server->getId());
        info->set_host(server->getAddress());
        info->set_port(server->getPort());
        info->set_ishealthy(server->isHealthy());
    }

    return ::grpc::Status::OK;
}

::grpc::Status AdminService::UpdateServerHealth(
    ::grpc::ServerContext* context,
    const admin::UpdateServerHealthRequest* request,
    ::google::protobuf::Empty* response)
{
    // If you have some method to find a server by ID, do that here:
    auto server = server_manager_->findServerById(request->id()); 
    if (!server) {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              "Server not found");
    }

    server->setHealthStatus(request->ishealthy());
    return ::grpc::Status::OK;
}

::grpc::Status AdminService::AddServer(
    ::grpc::ServerContext* context,
    const ::google::protobuf::Empty* request,
    admin::AddServerResponse* response)
{
    bool success = server_manager_->addServer();
    if (!success) {
        return ::grpc::Status(::grpc::StatusCode::RESOURCE_EXHAUSTED,
                              "Max servers reached or cannot add");
    }
    // If you want the ID of the newly created server, 
    // you might have server_manager_->addServer() return the shared_ptr<Server> 
    // or you could track the last one created.
    // For example:
    auto latestServer = server_manager_->addServerAndReturn();
    if (latestServer) {
        response->set_id(latestServer->getId());
    }
    return ::grpc::Status::OK;
}

::grpc::Status AdminService::RemoveServer(
    ::grpc::ServerContext* context,
    const admin::RemoveServerRequest* request,
    ::google::protobuf::Empty* response)
{
    // For removing by ID, we might need a new method like removeServerById()
    bool success = server_manager_->removeServerById(request->id());
    if (!success) {
        return ::grpc::Status(::grpc::StatusCode::NOT_FOUND,
                              "Could not remove server (not found or min servers reached)");
    }
    return ::grpc::Status::OK;
}
