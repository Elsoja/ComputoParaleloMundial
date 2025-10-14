#include "controller/TeamController.hpp"
#include "configuration/RouteDefinition.hpp"
#include "domain/Utilities.hpp"

// La implementación del constructor se queda aquí
TeamController::TeamController(const std::shared_ptr<ITeamDelegate>& teamDelegate) : teamDelegate(teamDelegate) {}

// La implementación de getTeam se queda aquí
crow::response TeamController::getTeam(const std::string& teamId) const {
    if(auto team = teamDelegate->GetTeam(teamId); team != nullptr) {
        nlohmann::json body = *team;
        auto response = crow::response{crow::OK, body.dump()};
        response.add_header("Content-Type", "application/json");
        return response;
    }
    return crow::response{crow::NOT_FOUND, "{\"error\":\"Team not found\"}"};
}

// La implementación de getAllTeams se queda aquí
crow::response TeamController::getAllTeams() const {
    nlohmann::json body = teamDelegate->GetAllTeams();
    auto response = crow::response{crow::OK, body.dump()};
    response.add_header("Content-Type", "application/json");
    return response;
}

// CAMBIO: El cuerpo de SaveTeam se ha movido al archivo .hpp

REGISTER_ROUTE(TeamController, getTeam, "/teams/<string>", "GET"_method)
REGISTER_ROUTE(TeamController, getAllTeams, "/teams", "GET"_method)
REGISTER_ROUTE(TeamController, SaveTeam, "/teams", "POST"_method)