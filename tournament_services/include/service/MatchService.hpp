#ifndef MATCH_SERVICE_HPP
#define MATCH_SERVICE_HPP

#include "domain/Match.hpp"
#include "persistence/repository/IMatchRepository.hpp"
#include "events/Events.hpp"
#include <memory>
#include <vector>
#include <string>

namespace service {

class MatchService {
private:
    std::shared_ptr<repository::IMatchRepository> matchRepository;

public:
    explicit MatchService(std::shared_ptr<repository::IMatchRepository> matchRepo);
    
    void RegisterMatchResult(const std::string& matchId, int team1Score, int team2Score);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByTournament(const std::string& tournamentId);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByPhase(const std::string& tournamentId, domain::MatchPhase phase);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByGroup(const std::string& groupId);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByTeam(const std::string& teamId);
    
    std::shared_ptr<domain::Match> GetMatchById(const std::string& matchId);
    
    domain::Match CreateMatch(const domain::Match& match);
    
    void UpdateMatch(const domain::Match& match);
    
    void DeleteMatch(const std::string& matchId);

private:
    std::string MatchPhaseToString(domain::MatchPhase phase);
};

} // namespace service

#endif // MATCH_SERVICE_HPP