#include "api/crow_service.hpp"
#include <iostream>

// Helper function to add CORS headers to a response
static void addCORSHeaders(crow::response& res)
{
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type");
}

// The main function that runs your Crow HTTP server
void runCrowServer(std::shared_ptr<ServerManager> server_manager)
{
    crow::SimpleApp app;

    // -----------------------------------------------------------------------
    // 1) /api/status (GET, OPTIONS)
    // -----------------------------------------------------------------------
    CROW_ROUTE(app, "/api/status")
        .methods("GET"_method, "OPTIONS"_method)
    ([server_manager](const crow::request& req, crow::response& res)
    {
        // Always set CORS headers
        addCORSHeaders(res);

        // If it's an OPTIONS request, just return 200 OK now
        if (req.method == crow::HTTPMethod::OPTIONS)
        {
            res.end();
            return;
        }

        // Otherwise, it's a GET
        nlohmann::json json_response;
        json_response["servers"] = nlohmann::json::array();

        // Gather info from ServerManager
        auto servers = server_manager->getAllServers();
        for (const auto& server : servers) {
            json_response["servers"].push_back({
                {"id",       server->getId()},
                {"host",     server->getAddress()},
                {"port",     server->getPort()},
                {"healthy",  server->isHealthy()},
                {"requests", server->getRequestCount()}
            });
        }

        // Send JSON body
        res.write(json_response.dump());
        res.end();
    });

    // -----------------------------------------------------------------------
    // 2) /api/add_server (POST, OPTIONS)
    // -----------------------------------------------------------------------
    CROW_ROUTE(app, "/api/add_server")
        .methods("POST"_method, "OPTIONS"_method)
    ([server_manager](const crow::request& req, crow::response& res)
    {
        addCORSHeaders(res);

        if (req.method == crow::HTTPMethod::OPTIONS)
        {
            res.end();
            return;
        }

        // POST logic to add a new server
        auto newSrv = server_manager->addServer();
        if (!newSrv) {
            // E.g. max servers reached
            res.code = 400;
            res.write(R"({"error": "Max servers reached or cannot add"})");
            res.end();
            return;
        }

        // Success JSON
        nlohmann::json j{
            {"message", "Server added successfully"},
            {"id", newSrv->getId()}
        };
        res.write(j.dump());
        res.end();
    });


    // -----------------------------------------------------------------------
    // 3) /api/remove_server (POST, OPTIONS)
    // -----------------------------------------------------------------------
    CROW_ROUTE(app, "/api/remove_server")
        .methods("POST"_method, "OPTIONS"_method)
    ([server_manager](const crow::request& req, crow::response& res)
    {
        addCORSHeaders(res);

        if (req.method == crow::HTTPMethod::OPTIONS)
        {
            res.end();
            return;
        }

        // Parse JSON body to find "id"
        try {
            auto body = nlohmann::json::parse(req.body);
            if (!body.contains("id") || !body["id"].is_string()) {
                res.code = 400;
                res.write(R"({"error": "Missing or invalid server ID"})");
                res.end();
                return;
            }

            std::string server_id = body["id"].get<std::string>();
            std::cout << "Removing server with ID: " << server_id << std::endl;

            // Attempt removal
            if (!server_manager->removeServerById(server_id)) {
                // Possibly server not found or min servers
                res.code = 404;
                res.write(R"({"error": "Server not found or minimum servers reached"})");
                res.end();
                return;
            }

            // Success response
            nlohmann::json j{
                {"message", "Server removed successfully"},
                {"id", server_id}
            };
            res.write(j.dump());
            res.end();

        } catch (const nlohmann::json::parse_error&) {
            res.code = 400;
            res.write(R"({"error": "Invalid JSON payload"})");
            res.end();
        }
    });

    // -----------------------------------------------------------------------
    // Finally, start the HTTP server on port 8080
    // -----------------------------------------------------------------------
    std::cout << "Starting Crow HTTP API on port 8080..." << std::endl;
    app.port(8080).multithreaded().run();
}
