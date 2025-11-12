#ifndef MATCH_CONTROLLER_HPP
#define MATCH_CONTROLLER_HPP

#include "service/MatchService.hpp" // Depende del Servicio
#include <crow.h>
#include <memory>
#include <string>

// Declaraciones anticipadas para una compilación más limpia
namespace service {
    class MatchService;
}
namespace crow {
    struct request;
    struct response;
}

// ✅ AGREGADO: Namespace controller
namespace controller {

class MatchController {
    std::shared_ptr<service::MatchService> matchService;
public:
    explicit MatchController(std::shared_ptr<service::MatchService> service);
    
    // --- Endpoints del CRUD ---

    // POST /matches/{id}/score - Registrar puntaje
    crow::response RegisterScore(const crow::request& req, const std::string& matchId) const;

    // GET /matches/{id} - Obtener un partido por ID
    crow::response GetMatchById(const std::string& matchId) const;

    // GET /tournaments/{id}/matches - Obtener partidos de un torneo
    crow::response GetMatchesByTournament(const std::string& tournamentId) const;

    // GET /tournaments/{id}/matches/phase/{phase} - Obtener partidos por fase
    crow::response GetMatchesByPhase(const std::string& tournamentId, const std::string& phase) const;

    // GET /groups/{id}/matches - Obtener partidos de un grupo
    crow::response GetMatchesByGroup(const std::string& groupId) const;

    // GET /teams/{id}/matches - Obtener partidos de un equipo
    crow::response GetMatchesByTeam(const std::string& teamId) const;
    
    // POST /api/matches - Crear un partido (Uso administrativo/interno)
    crow::response CreateMatch(const crow::request& req) const;

    // PUT /api/matches/{id} - Actualizar un partido (Uso administrativo/interno)
    crow::response UpdateMatch(const crow::request& req, const std::string& matchId) const;

    // DELETE /api/matches/{id} - Eliminar un partido (Uso administrativo/interno)
    crow::response DeleteMatch(const std::string& matchId) const;
};

} // namespace controller

#endif // MATCH_CONTROLLER_HPP