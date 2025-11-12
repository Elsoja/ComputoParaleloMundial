#include "controller/MatchController.hpp"
#include "configuration/RouteDefinition.hpp"
#include "domain/Match.hpp"
#include <nlohmann/json.hpp>
#include <utility>

// ✅ CAMBIO: Implementación DENTRO del namespace
namespace controller { 

MatchController::MatchController(std::shared_ptr<service::MatchService> service)
    : matchService(std::move(service)) {}

// POST /api/matches/{id}/score
crow::response MatchController::RegisterScore(const crow::request& req, const std::string& matchId) const {
    try {
        auto body = nlohmann::json::parse(req.body);
        int scoreA = body.at("scoreA").get<int>();
        int scoreB = body.at("scoreB").get<int>();
        
        matchService->RegisterMatchResult(matchId, scoreA, scoreB);
        
        return crow::response(crow::NO_CONTENT);
    } 
    catch (const nlohmann::json::exception& e) {
        return crow::response(crow::BAD_REQUEST, std::string("{\"error\":\"JSON Inválido: ") + e.what() + "\"}");
    }
    catch (const std::exception& ex) { // Renombrado para evitar shadowing
        return crow::response(crow::NOT_FOUND, std::string("{\"error\":\"") + ex.what() + "\"}");
    }
}

// GET /api/matches/{id}
crow::response MatchController::GetMatchById(const std::string& matchId) const {
    auto match = matchService->GetMatchById(matchId);
    if (match) {
        nlohmann::json responseBody = *match; // Renombrado para evitar shadowing
        return crow::response(crow::OK, responseBody.dump());
    }
    return crow::response(crow::NOT_FOUND, "{\"error\":\"Match not found\"}");
}

// GET /api/tournaments/{id}/matches
crow::response MatchController::GetMatchesByTournament(const std::string& tournamentId) const {
    auto matchList = matchService->GetMatchesByTournament(tournamentId); // Renombrado
    nlohmann::json responseBody = matchList; // Renombrado
    return crow::response(crow::OK, responseBody.dump());
}

// GET /api/tournaments/{id}/matches/phase/{phase}
crow::response MatchController::GetMatchesByPhase(const std::string& tournamentId, const std::string& phase) const {
    try {
        domain::MatchPhase phaseEnum = domain::Match::StringToPhase(phase);
        auto matchList = matchService->GetMatchesByPhase(tournamentId, phaseEnum); // Renombrado
        nlohmann::json responseBody = matchList; // Renombrado
        return crow::response(crow::OK, responseBody.dump());
    } catch (const std::exception& e) {
        return crow::response(crow::BAD_REQUEST, std::string("{\"error\":\"Fase inválida: ") + e.what() + "\"}");
    }
}

// GET /api/groups/{id}/matches
crow::response MatchController::GetMatchesByGroup(const std::string& groupId) const {
    auto matchList = matchService->GetMatchesByGroup(groupId); // Renombrado
    nlohmann::json responseBody = matchList; // Renombrado
    return crow::response(crow::OK, responseBody.dump());
}

// GET /api/teams/{id}/matches
crow::response MatchController::GetMatchesByTeam(const std::string& teamId) const {
    auto matchList = matchService->GetMatchesByTeam(teamId); // Renombrado
    nlohmann::json responseBody = matchList; // Renombrado
    return crow::response(crow::OK, responseBody.dump());
}

// POST /api/matches (Uso administrativo)
crow::response MatchController::CreateMatch(const crow::request& req) const {
    try {
        auto body = nlohmann::json::parse(req.body);
        domain::Match match;
        from_json(body, match);
        auto createdMatch = matchService->CreateMatch(match);
        nlohmann::json responseBody = createdMatch;
        return crow::response(crow::CREATED, responseBody.dump());
    } catch (const std::exception& e) {
        return crow::response(crow::BAD_REQUEST, std::string("{\"error\":\"") + e.what() + "\"}");
    }
}

// PUT /api/matches/{id} (Uso administrativo)
crow::response MatchController::UpdateMatch(const crow::request& req, const std::string& matchId) const {
    try {
        auto body = nlohmann::json::parse(req.body);
        domain::Match match;
        from_json(body, match);
        match.Id() = matchId; 

        matchService->UpdateMatch(match);
        return crow::response(crow::NO_CONTENT);
    } catch (const std::exception& e) {
        return crow::response(crow::BAD_REQUEST, std::string("{\"error\":\"") + e.what() + "\"}");
    }
}

// DELETE /api/matches/{id} (Uso administrativo)
crow::response MatchController::DeleteMatch(const std::string& matchId) const {
    try {
        matchService->DeleteMatch(matchId);
        return crow::response(crow::NO_CONTENT);
    } catch (const std::exception& e) {
        return crow::response(crow::NOT_FOUND, std::string("{\"error\":\"") + e.what() + "\"}");
    }
}


// --- Registrar todas las rutas ---
REGISTER_ROUTE(MatchController, RegisterScore, "/api/matches/<string>/score", "POST"_method)
REGISTER_ROUTE(MatchController, GetMatchesByTournament, "/api/tournaments/<string>/matches", "GET"_method)
REGISTER_ROUTE(MatchController, GetMatchesByPhase, "/api/tournaments/<string>/matches/phase/<string>", "GET"_method)
REGISTER_ROUTE(MatchController, GetMatchesByGroup, "/api/groups/<string>/matches", "GET"_method)
REGISTER_ROUTE(MatchController, GetMatchesByTeam, "/api/teams/<string>/matches", "GET"_method)
REGISTER_ROUTE(MatchController, GetMatchById, "/api/matches/<string>", "GET"_method)
REGISTER_ROUTE(MatchController, CreateMatch, "/api/matches", "POST"_method)
REGISTER_ROUTE(MatchController, UpdateMatch, "/api/matches/<string>", "PUT"_method)
REGISTER_ROUTE(MatchController, DeleteMatch, "/api/matches/<string>", "DELETE"_method)

} // namespace controller