#ifndef RESTAPI_TEAMCONTROLLER_HPP
#define RESTAPI_TEAMCONTROLLER_HPP

#include "delegate/ITeamDelegate.hpp"
#include "domain/Team.hpp"
#include <nlohmann/json.hpp>
#include "crow.h"
#include <memory>
#include <string>

class TeamController {
    std::shared_ptr<ITeamDelegate> teamDelegate;
public:
    explicit TeamController(const std::shared_ptr<ITeamDelegate>& delegate);

    crow::response getTeam(const std::string& teamId) const;
    crow::response getAllTeams() const;
    crow::response UpdateTeam(const crow::request& request, const std::string& id) const;
    crow::response DeleteTeam(const std::string& id) const;

    // La implementación de SaveTeam vive aquí como 'inline'
    inline crow::response SaveTeam(const crow::request& request) const {
        if (!nlohmann::json::accept(request.body)) {
            return crow::response{crow::BAD_REQUEST, "{\"error\":\"Invalid JSON format\"}"};
        }
        try {
            auto requestBody = nlohmann::json::parse(request.body);
            domain::Team team;
            from_json(requestBody, team);

            auto result = teamDelegate->SaveTeam(team);

            if (result) {
                crow::response response(crow::CREATED);
                response.add_header("Location", result.value());
                return response;
            } else {
                return crow::response{crow::CONFLICT, "{\"error\":\"Team already exists\"}"};
            }
        } catch (const nlohmann::json::exception& e) {
            return crow::response{crow::BAD_REQUEST, "{\"error\":\"JSON parsing error\"}"};
        }
    }
};

#endif // RESTAPI_TEAMCONTROLLER_HPP