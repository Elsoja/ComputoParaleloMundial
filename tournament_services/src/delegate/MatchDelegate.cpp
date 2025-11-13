#include "delegate/MatchDelegate.hpp"
#include "domain/Match.hpp"
#include <stdexcept>
#include <string>

namespace delegate {

MatchDelegate::MatchDelegate(std::shared_ptr<service::MatchService> service)
    : matchService(std::move(service)) {}

void MatchDelegate::RegisterScore(const std::string& matchId, int team1Score, int team2Score) {
    auto matchOpt = matchService->GetMatchById(matchId); 
    if (!matchOpt) {
        throw std::runtime_error("Partido no encontrado");
    }
        
    matchService->RegisterMatchResult(matchId, team1Score, team2Score);
}

std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByTournament(const std::string& tournamentId) {
    return matchService->GetMatchesByTournament(tournamentId);
}

std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByPhase(const std::string& tournamentId, const std::string& phase) {
    domain::MatchPhase matchPhase = domain::Match::StringToPhase(phase);
    return matchService->GetMatchesByPhase(tournamentId, matchPhase);
}

std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByGroup(const std::string& groupId) {
    return matchService->GetMatchesByGroup(groupId);
}

std::vector<std::shared_ptr<domain::Match>> MatchDelegate::GetMatchesByTeam(const std::string& teamId) {
    return matchService->GetMatchesByTeam(teamId);
}

std::shared_ptr<domain::Match> MatchDelegate::GetMatchById(const std::string& matchId) {
    return matchService->GetMatchById(matchId);
}

domain::Match MatchDelegate::CreateMatch(const domain::Match& match) {
    return matchService->CreateMatch(match);
}

void MatchDelegate::UpdateMatch(const domain::Match& match) {
    matchService->UpdateMatch(match);
}

void MatchDelegate::DeleteMatch(const std::string& matchId) {
    matchService->DeleteMatch(matchId);
}

} // namespace delegate