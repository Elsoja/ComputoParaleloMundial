#ifndef TOURNAMENTS_IMATCHREPOSITORY_HPP
#define TOURNAMENTS_IMATCHREPOSITORY_HPP

#include "persistence/repository/IRepository.hpp"
#include "domain/Match.hpp" // Incluir Match
#include <vector>
#include <string>
#include <memory>


namespace repository { 

// Usar 'domain::Match' directamente
class IMatchRepository : public IRepository<domain::Match, std::string> {
public:
    virtual ~IMatchRepository() = default;
    
    virtual std::vector<std::shared_ptr<domain::Match>> FindByTournamentId(std::string tournamentId) = 0;
    // Usar 'domain::MatchPhase'
    virtual std::vector<std::shared_ptr<domain::Match>> FindByTournamentIdAndPhase(std::string tournamentId, domain::MatchPhase phase) = 0;
    virtual std::vector<std::shared_ptr<domain::Match>> FindByGroupId(std::string groupId) = 0;
    virtual std::vector<std::shared_ptr<domain::Match>> FindByTeamId(std::string teamId) = 0;
    virtual bool IsGroupStageComplete(std::string tournamentId) = 0;
    virtual domain::Match Save(const domain::Match& match) = 0;
};
}
#endif