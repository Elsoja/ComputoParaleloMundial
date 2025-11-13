#ifndef MATCH_DELEGATE_HPP
#define MATCH_DELEGATE_HPP

#include "service/MatchService.hpp" // Depende de MatchService
#include "domain/Match.hpp"
#include <memory>
#include <vector>
#include <string>

namespace delegate {

class MatchDelegate {
private:
    std::shared_ptr<service::MatchService> matchService;

public:
    explicit MatchDelegate(std::shared_ptr<service::MatchService> service);
    ~MatchDelegate() = default;

    
    void RegisterScore(const std::string& matchId, int team1Score, int team2Score);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByTournament(const std::string& tournamentId);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByPhase(const std::string& tournamentId, const std::string& phase);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByGroup(const std::string& groupId);
    
    std::vector<std::shared_ptr<domain::Match>> GetMatchesByTeam(const std::string& teamId);
    
    std::shared_ptr<domain::Match> GetMatchById(const std::string& matchId);
    
    domain::Match CreateMatch(const domain::Match& match);
    
    void UpdateMatch(const domain::Match& match);
    
    void DeleteMatch(const std::string& matchId);
};

} // namespace delegate

#endif // MATCH_DELEGATE_HPP