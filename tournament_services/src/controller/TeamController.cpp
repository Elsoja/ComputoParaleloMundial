#include "controller/TeamController.hpp"
#include "configuration/RouteDefinition.hpp"
#include "domain/Utilities.hpp" 

// La implementación del constructor
TeamController::TeamController(const std::shared_ptr<ITeamDelegate>& delegate) : teamDelegate(delegate) {}

// La implementación de getTeam
crow::response TeamController::getTeam(const std::string& teamId) const {
    if(auto team = teamDelegate->GetTeam(teamId); team != nullptr) {
        nlohmann::json body = *team;
        auto response = crow::response{crow::OK, body.dump()};
        response.add_header("Content-Type", "application/json");
        return response;
    }
    return crow::response{crow::NOT_FOUND, "{\"error\":\"Team not found\"}"};
}

// La implementación de getAllTeams
crow::response TeamController::getAllTeams() const {
    nlohmann::json body = teamDelegate->GetAllTeams();
    auto response = crow::response{crow::OK, body.dump()};
    response.add_header("Content-Type", "application/json");
    return response;
}

// La implementación de UpdateTeam
crow::response TeamController::UpdateTeam(const crow::request& request, const std::string& id) const {
    auto requestBody = nlohmann::json::parse(request.body);
    domain::Team team;
    from_json(requestBody, team);
    team.Id() = id;

    auto result = teamDelegate->UpdateTeam(id, team);
    
    if(result) {
        return crow::response(crow::NO_CONTENT); // 204 No Content
    } else {
        return crow::response(crow::NOT_FOUND); // 404 Not Found
    }
} 

// La implementación de DeleteTeam
crow::response TeamController::DeleteTeam(const std::string& id) const {
    auto result = teamDelegate->DeleteTeam(id);

    if (result) {
        return crow::response(crow::NO_CONTENT); // 204 No Content
    } else {
        return crow::response(crow::NOT_FOUND); // 404 Not Found
    }
}

// El registro de TODAS las rutas vive aquí
REGISTER_ROUTE(TeamController, getTeam, "/teams/<string>", "GET"_method)
REGISTER_ROUTE(TeamController, getAllTeams, "/teams", "GET"_method)
REGISTER_ROUTE(TeamController, SaveTeam, "/teams", "POST"_method)
REGISTER_ROUTE(TeamController, UpdateTeam, "/teams/<string>", "PATCH"_method)
REGISTER_ROUTE(TeamController, DeleteTeam, "/teams/<string>", "DELETE"_method)