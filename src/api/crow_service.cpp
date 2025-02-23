#include "api/crow_service.hpp"

void runCrowServer(std::shared_ptr<ServerManager> server_manager) {
    crow::SimpleApp app;

    // Status endpoint
    CROW_ROUTE(app, "/api/status").methods(crow::HTTPMethod::Get)
    ([server_manager]() -> crow::response {
        nlohmann::json json_response;
        auto servers = server_manager->getAllServers();

        json_response["servers"] = nlohmann::json::array();
        for (const auto& server : servers) {
            json_response["servers"].push_back({
                {"id", server->getId()},
                {"host", server->getAddress()},
                {"port", server->getPort()},
                {"healthy", server->isHealthy()},
                {"requests", server->getRequestCount()}
            });
        }
        return crow::response(json_response.dump());
    });

    // Add server endpoint
    CROW_ROUTE(app, "/api/add_server").methods(crow::HTTPMethod::Post)
    ([server_manager]() -> crow::response {
        auto newServer = server_manager->addServer();
        if (!newServer) {
            return crow::response(400, R"({"error": "Max servers reached or cannot add"})");
        }

        nlohmann::json response_json{
            {"message", "Server added successfully"},
            {"id", newServer->getId()}
        };
        return crow::response(200, response_json.dump());
    });

    // Remove server endpoint
    CROW_ROUTE(app, "/api/remove_server").methods(crow::HTTPMethod::Post)
    ([server_manager](const crow::request& req) -> crow::response {
        try {
            auto body = nlohmann::json::parse(req.body);
            if (!body.contains("id") || !body["id"].is_string()) {
                return crow::response(400, R"({"error": "Missing or invalid server ID"})");
            }

            std::string server_id = body["id"];
            std::cout<<"Removing server with id: "<<server_id<<std::endl;
            if (!server_manager->removeServerById(server_id)) {
                return crow::response(404, R"({"error": "Server not found or minimum servers reached"})");
            }

            nlohmann::json response_json{
                {"message", "Server removed successfully"},
                {"id", server_id}
            };
            return crow::response(200, response_json.dump());
        } catch (const nlohmann::json::parse_error& e) {
            return crow::response(400, R"({"error": "Invalid JSON payload"})");
        }
    });

    std::cout << "Starting Crow HTTP API on port 8080..." << std::endl;
    app.port(8080).multithreaded().run();
}