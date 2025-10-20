#include "controller/TournamentController.hpp"
#include "configuration/RouteDefinition.hpp"
#include "domain/Tournament.hpp"
#include "delegate/ITournamentDelegate.hpp"
#include <nlohmann/json.hpp>
#include <utility>

TournamentController::TournamentController(std::shared_ptr<ITournamentDelegate> delegate) : tournamentDelegate(std::move(delegate)) {}

crow::response TournamentController::CreateTournament(const crow::request& request) const {
    if (!nlohmann::json::accept(request.body)) {
        return crow::response{crow::BAD_REQUEST, "{\"error\":\"Invalid JSON format\"}"};
    }
    try {
        auto requestBody = nlohmann::json::parse(request.body);
        auto tournament = std::make_shared<domain::Tournament>();
        from_json(requestBody, *tournament);

        auto result = tournamentDelegate->CreateTournament(tournament);

        if (result) {
            crow::response res(crow::CREATED);
            res.set_header("Location", result.value());
            return res;
        } else {
            return crow::response{crow::CONFLICT, "{\"error\":\"Tournament already exists\"}"};
        }
    } catch (const nlohmann::json::exception& e) {
        return crow::response{crow::BAD_REQUEST, "{\"error\":\"JSON parsing error\"}"};
    }
}

crow::response TournamentController::ReadAll() const {
    nlohmann::json body = tournamentDelegate->GetAllTournaments();
    crow::response response(crow::OK);
    response.add_header("Content-Type", "application/json");
    response.body = body.dump();
    return response;
}

crow::response TournamentController::GetTournament(const std::string& id) const {
    auto tournamentPtr = tournamentDelegate->GetTournament(id);
    if (tournamentPtr != nullptr) {
        nlohmann::json body = *tournamentPtr;
        return crow::response(crow::OK, body.dump());
    }
    return crow::response(crow::NOT_FOUND);
}

crow::response TournamentController::UpdateTournament(const crow::request& request, const std::string& id) const {
    try {
        auto body = nlohmann::json::parse(request.body);
        domain::Tournament tournament;
        from_json(body, tournament);
        tournament.Id() = id;

        auto result = tournamentDelegate->UpdateTournament(id, tournament);
        if (result) {
            return crow::response(crow::NO_CONTENT);
        }
        return crow::response(crow::NOT_FOUND);
    } catch (const nlohmann::json::exception& e) {
        return crow::response{crow::BAD_REQUEST, "{\"error\":\"JSON parsing error\"}"};
    }
}

crow::response TournamentController::DeleteTournament(const std::string& id) const {
    auto result = tournamentDelegate->DeleteTournament(id);
    if (result) {
        return crow::response(crow::NO_CONTENT);
    }
    return crow::response(crow::NOT_FOUND);
}

// Se registran todas las rutas para los métodos del TournamentController
REGISTER_ROUTE(TournamentController, CreateTournament, "/tournaments", "POST"_method)
REGISTER_ROUTE(TournamentController, ReadAll, "/tournaments", "GET"_method)
REGISTER_ROUTE(TournamentController, GetTournament, "/tournaments/<string>", "GET"_method)
REGISTER_ROUTE(TournamentController, UpdateTournament, "/tournaments/<string>", "PATCH"_method)
REGISTER_ROUTE(TournamentController, DeleteTournament, "/tournaments/<string>", "DELETE"_method)