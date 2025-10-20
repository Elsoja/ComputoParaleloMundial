#include "controller/GroupController.hpp"
#include "configuration/RouteDefinition.hpp"
#include "domain/Group.hpp"
#include <nlohmann/json.hpp>
#include <utility>

GroupController::GroupController(const std::shared_ptr<IGroupDelegate>& delegate) : groupDelegate(std::move(delegate)) {}

crow::response GroupController::CreateGroup(const crow::request& request, const std::string& tournamentId) const {
    if (!nlohmann::json::accept(request.body)) {
        return crow::response(crow::BAD_REQUEST, "{\"error\":\"Invalid JSON format\"}");
    }
    try {
        auto body = nlohmann::json::parse(request.body);
        domain::Group group;
        from_json(body, group);
        auto result = groupDelegate->CreateGroup(tournamentId, group);
        if (result) {
            crow::response res(crow::CREATED);
            res.add_header("Location", result.value());
            return res;
        } else {
            if (result.error() == "Tournament not found") {
                return crow::response(crow::NOT_FOUND, "{\"error\":\"" + result.error() + "\"}");
            }
            // ✅ CAMBIO: Se usa el número 422 en lugar de la constante inexistente.
            return crow::response(422, "{\"error\":\"" + result.error() + "\"}");
        }
    } catch (const nlohmann::json::exception& e) {
       return crow::response(crow::BAD_REQUEST, "{\"error\":\"JSON parsing error\"}");
    }
}

crow::response GroupController::GetGroups(const std::string& tournamentId) const {
    auto result = groupDelegate->GetGroups(tournamentId);
    if (result) {
        nlohmann::json body = result.value();
        return crow::response(crow::OK, body.dump());
    }
    return crow::response(crow::INTERNAL_SERVER_ERROR, "{\"error\":\"" + result.error() + "\"}");
}

crow::response GroupController::GetGroup(const std::string& tournamentId, const std::string& groupId) const {
    auto result = groupDelegate->GetGroup(tournamentId, groupId);
    if (result) {
        nlohmann::json body = result.value();
        return crow::response(crow::OK, body.dump());
    }
    return crow::response(crow::NOT_FOUND);
}

crow::response GroupController::UpdateGroup(const crow::request& request, const std::string& tournamentId, const std::string& groupId) const {
    auto body = nlohmann::json::parse(request.body);
    domain::Group group;
    from_json(body, group);
    group.Id() = groupId;

    auto result = groupDelegate->UpdateGroup(tournamentId, group);
    if (result) {
        return crow::response(crow::NO_CONTENT);
    }
    return crow::response(crow::NOT_FOUND);
}

crow::response GroupController::DeleteGroup(const std::string& tournamentId, const std::string& groupId) const {
    auto result = groupDelegate->RemoveGroup(tournamentId, groupId);
    if (result) {
        return crow::response(crow::NO_CONTENT);
    }
    return crow::response(crow::NOT_FOUND);
}

// CAMBIO: El registro de rutas ahora vive únicamente en este archivo .cpp
REGISTER_ROUTE(GroupController, CreateGroup, "/tournaments/<string>/groups", "POST"_method)
REGISTER_ROUTE(GroupController, GetGroups, "/tournaments/<string>/groups", "GET"_method)
REGISTER_ROUTE(GroupController, GetGroup, "/tournaments/<string>/groups/<string>", "GET"_method)
REGISTER_ROUTE(GroupController, UpdateGroup, "/tournaments/<string>/groups/<string>", "PATCH"_method)
REGISTER_ROUTE(GroupController, DeleteGroup, "/tournaments/<string>/groups/<string>", "DELETE"_method)