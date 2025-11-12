#ifndef MATCH_EVENT_HANDLER_HPP
#define MATCH_EVENT_HANDLER_HPP

#include "events/Events.hpp"
#include "domain/Match.hpp"
#include "domain/Group.hpp"
#include "domain/IMatchStrategy.hpp"
#include "persistence/repository/IMatchRepository.hpp"
#include <memory>
#include <vector>
#include <map>
#include <algorithm>

namespace handlers {

class MatchEventHandler {
private:
    std::shared_ptr<repository::IMatchRepository> matchRepository;

    // Estructura para almacenar estadísticas de un equipo en el grupo
    struct TeamStanding {
        std::string teamId;
        int wins;
        int pointsFor;
        int pointsAgainst;  
        int draws;
        int losses;
        int matchesPlayed;

        bool operator<(const TeamStanding& other) const {
            // 1° Más victorias
            if (wins != other.wins) return wins > other.wins;
            
            // 2° Más puntos a favor
            if (pointsFor != other.pointsFor) return pointsFor > other.pointsFor;
            
            // 3° Menos puntos en contra
            return pointsAgainst < other.pointsAgainst;
        }
    };

    domain::Match CreateMatchWithTeams(
        const std::string& tournamentId, 
        domain::MatchPhase phase, 
        int matchNumber,
        const std::string& team1Id, 
        const std::string& team2Id
    );

public:
    explicit MatchEventHandler(std::shared_ptr<repository::IMatchRepository> matchRepo);

    // Suscribirse a los eventos
    void Subscribe();

    // Handler principal: Se registró un puntaje
    void OnScoreRegistered(const events::ScoreRegisteredEvent& event);

private:
    // Manejar puntaje registrado en fase de grupos
    void HandleGroupStageScoreRegistered(const events::ScoreRegisteredEvent& event);

    // Manejar puntaje registrado en fase de playoffs
    void HandlePlayoffScoreRegistered(const events::ScoreRegisteredEvent& event);

    // Verificar si todos los partidos de grupo están completos
    bool IsGroupStageComplete(const std::string& tournamentId);

    // Generar partidos de playoffs desde la fase de grupos
    void GeneratePlayoffsFromGroupStage(const std::string& tournamentId);

    // Aplicar reglas de tabulación para obtener equipos clasificados
    std::vector<std::string> ApplyStandingsRules(const std::string& tournamentId, const std::vector<std::string>& groupIds);

    // Calcular tabla de posiciones de un grupo
    std::vector<TeamStanding> CalculateGroupStandings(const std::string& groupId);

    // Generar partidos de playoffs con equipos clasificados
    std::vector<domain::Match> GeneratePlayoffMatchesFromQualified(
        const std::vector<std::string>& qualifiedTeams,
        const std::string& tournamentId,
        domain::IMatchStrategy* strategy
    );

    // Actualizar el siguiente partido con el ganador
    void UpdateNextMatchWithWinner(
        const domain::Match& completedMatch,
        const std::string& winnerId,
        domain::IMatchStrategy* strategy
    );

    // Crear y asignar el siguiente partido
    void CreateAndAssignNextMatch(
        const domain::Match& completedMatch,
        const std::string& winnerId,
        domain::IMatchStrategy* strategy
    );

    // Determinar fase inicial según número de equipos
    domain::MatchPhase DetermineInitialPhase(int teamCount) const;

    // Obtener la siguiente fase
    domain::MatchPhase GetNextPhase(domain::MatchPhase currentPhase) const;

    // Generar estructura vacía de playoffs
    void GenerateEmptyPlayoffStructure(
        std::vector<domain::Match>& matches,
        const std::string& tournamentId,
        domain::MatchPhase initialPhase,
        int teamCount
    );

    // Vincular partidos de playoffs
    void LinkPlayoffMatches(std::vector<domain::Match>& matches);

    // Obtener IDs de grupos de un torneo (simulado por ahora)
    std::vector<std::string> GetGroupIdsForTournament(const std::string& tournamentId);
};

} // namespace handlers

#endif // MATCH_EVENT_HANDLER_HPP