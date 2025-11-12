#include "service/MatchService.hpp"
#include <stdexcept>
#include <string>

namespace service {

MatchService::MatchService(std::shared_ptr<repository::IMatchRepository> matchRepo)
    : matchRepository(std::move(matchRepo)) {}

// ✅ CAMBIO: ID es const std::string&
void MatchService::RegisterMatchResult(const std::string& matchId, int team1Score, int team2Score) {
    // ✅ CAMBIO: FindById -> ReadById
    auto match = matchRepository->ReadById(matchId); 
    if (!match) {
        throw std::runtime_error("Partido no encontrado");
    }

    if (!match->HasBothTeams()) {
        throw std::runtime_error("El partido no tiene ambos equipos asignados");
    }

    match->SetScore(team1Score, team2Score);
    matchRepository->Update(*match);

    std::string phaseStr = domain::Match::PhaseToString(match->Phase());
    std::string winnerId = match->WinnerId().value_or(""); // Manejar opcional

    events::ScoreRegisteredEvent event(
        matchId,
        match->TournamentId(),
        team1Score,
        team2Score,
        winnerId,
        phaseStr
    );
    events::EventBus::Instance()->Publish(event);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchService::GetMatchesByTournament(const std::string& tournamentId) {
    return matchRepository->FindByTournamentId(tournamentId);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchService::GetMatchesByPhase(const std::string& tournamentId, domain::MatchPhase phase) {
    return matchRepository->FindByTournamentIdAndPhase(tournamentId, phase);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchService::GetMatchesByGroup(const std::string& groupId) {
    return matchRepository->FindByGroupId(groupId);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchService::GetMatchesByTeam(const std::string& teamId) {
    return matchRepository->FindByTeamId(teamId);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::shared_ptr<domain::Match> MatchService::GetMatchById(const std::string& matchId) {
    // ✅ CAMBIO: FindById -> ReadById
    return matchRepository->ReadById(matchId);
}

domain::Match MatchService::CreateMatch(const domain::Match& match) {
    return matchRepository->Save(match);
}

void MatchService::UpdateMatch(const domain::Match& match) {
    matchRepository->Update(match);
}

// ✅ CAMBIO: Firma coincide con el .hpp
void MatchService::DeleteMatch(const std::string& matchId) {
    matchRepository->Delete(matchId);
}

std::string MatchService::MatchPhaseToString(domain::MatchPhase phase) {
    return domain::Match::PhaseToString(phase);
}

} // namespace service