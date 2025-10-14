//
// Created by tsuny on 8/31/25.
//

#include "configuration/RouteDefinition.hpp"
#include "controller/TournamentController.hpp"
#include "delegate/ITournamentDelegate.hpp" // CAMBIO: Incluir la interfaz para los tipos de error
#include "domain/Tournament.hpp"
#include <nlohmann/json.hpp> // CAMBIO: Incluir para el manejo de JSON
#include <string>
#include <utility>


TournamentController::TournamentController(std::shared_ptr<ITournamentDelegate> delegate) : tournamentDelegate(std::move(delegate)) {}

// CAMBIO: Lógica actualizada para manejar éxito y error
crow::response TournamentController::CreateTournament(const crow::request &request) const {
    if (!nlohmann::json::accept(request.body)) {
        return crow::response{crow::BAD_REQUEST, "{\"error\":\"Invalid JSON format\"}"};
    }

    try {
        auto requestBody = nlohmann::json::parse(request.body);
        auto tournament = std::make_shared<domain::Tournament>();
        from_json(requestBody, *tournament);

        // 1. Llamamos al delegate, que devuelve std::expected
        auto result = tournamentDelegate->CreateTournament(tournament);

        // 2. Verificamos el resultado
        if (result) {
            // ÉXITO: Devolvemos 201 Created con el ID en el header Location
            crow::response response(crow::CREATED);
            response.add_header("Location", result.value());
            return response;
        } else {
            // FRACASO: Hubo un conflicto, devolvemos 409 Conflict
            return crow::response{crow::CONFLICT, "{\"error\":\"Tournament already exists\"}"};
        }
    } catch (const nlohmann::json::exception& e) {
        return crow::response{crow::BAD_REQUEST, "{\"error\":\"JSON parsing error\"}"};
    }
}

crow::response TournamentController::ReadAll() const {
    // CAMBIO: Asegurarse de que el delegate devuelve un tipo compatible con JSON
    // y añadir el header Content-Type para consistencia.
    nlohmann::json body = tournamentDelegate->ReadAll();
    crow::response response(crow::OK);
    response.add_header("Content-Type", "application/json");
    response.body = body.dump();
    return response;
}


REGISTER_ROUTE(TournamentController, CreateTournament, "/tournaments", "POST"_method)
REGISTER_ROUTE(TournamentController, ReadAll, "/tournaments", "GET"_method)