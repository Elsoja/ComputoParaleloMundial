#include "delegate/MatchDelegate.hpp"
#include "domain/Match.hpp"
#include <stdexcept>
#include <string>

namespace delegate {

MatchDelegate::MatchDelegate(std::shared_ptr<service::MatchService> service)
    : matchService(std::move(service)) {}

// ✅ CAMBIO: ID es const std::string&
void MatchDelegate::RegisterScore(const std::string& matchId, int team1Score, int team2Score) {
    // ✅ CAMBIO: FindById -> GetMatchById (para coincidir con el servicio)
    auto matchOpt = matchService->GetMatchById(matchId); 
    if (!matchOpt) {
        throw std::runtime_error("Partido no encontrado");
    }
    
    // (Tu lógica de validación aquí si es necesaria)
    
    matchService->RegisterMatchResult(matchId, team1Score, team2Score);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByTournament(const std::string& tournamentId) {
    return matchService->GetMatchesByTournament(tournamentId);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByPhase(const std::string& tournamentId, const std::string& phase) {
    domain::MatchPhase matchPhase = domain::Match::StringToPhase(phase);
    return matchService->GetMatchesByPhase(tournamentId, matchPhase);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByGroup(const std::string& groupId) {
    return matchService->GetMatchesByGroup(groupId);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByTeam(const std::string& teamId) {
    return matchService->GetMatchesByTeam(teamId);
}

// ✅ CAMBIO: Firma y tipo de retorno coinciden con el .hpp
std::shared_ptr<domain::Match> MatchDelegate::GetMatchById(const std::string& matchId) {
    return matchService->GetMatchById(matchId);
}

domain::Match MatchDelegate::CreateMatch(const domain::Match& match) {
    return matchService->CreateMatch(match);
}

void MatchDelegate::UpdateMatch(const domain::Match& match) {
    matchService->UpdateMatch(match);
}

// ✅ CAMBIO: Firma coincide con el .hpp
void MatchDelegate::DeleteMatch(const std::string& matchId) {
    matchService->DeleteMatch(matchId);
}

} // namespace delegate