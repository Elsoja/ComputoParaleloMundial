#ifndef TOURNAMENTS_IMATCHSTRATEGY_HPP
#define TOURNAMENTS_IMATCHSTRATEGY_HPP

#include "domain/Match.hpp"
#include "domain/Team.hpp"
#include "domain/Group.hpp"
#include <vector>
#include <memory>
#include <optional>
#include <string>

namespace domain {

class IMatchStrategy {
public:
    virtual ~IMatchStrategy() = default;
    virtual std::vector<Match> GenerateGroupStageMatches(const std::vector<Group>& groups, const std::string& tournamentId) = 0;
    virtual std::vector<Match> GeneratePlayoffMatches(const std::vector<Group>& groups, const std::string& tournamentId) = 0;
    virtual std::optional<Match> GenerateNextMatch(
        const Match& completedMatch,
        const std::vector<std::shared_ptr<domain::Match>>& existingMatches 
    ) = 0;
    virtual void UpdateMatchWithWinner(Match& match, const std::string& winnerId, bool isTeam1) = 0;
    virtual bool AreGroupsFull(const std::vector<Group>& groups) const = 0;
};

// Declaración de la estrategia concreta
class SingleEliminationStrategy : public IMatchStrategy {
public:
    std::vector<Match> GenerateGroupStageMatches(const std::vector<Group>& groups, const std::string& tournamentId) override;
    std::vector<Match> GeneratePlayoffMatches(const std::vector<Group>& groups, const std::string& tournamentId) override;
    std::optional<Match> GenerateNextMatch(
        const Match& completedMatch,
        const std::vector<std::shared_ptr<domain::Match>>& existingMatches 
    ) override;
    void UpdateMatchWithWinner(Match& match, const std::string& winnerId, bool isTeam1) override;
    bool AreGroupsFull(const std::vector<Group>& groups) const override;
private:
    std::vector<std::string> GetQualifiedTeams(const std::vector<Group>& groups) const;
    MatchPhase DetermineInitialPhase(int teamCount) const;
    MatchPhase GetNextPhase(MatchPhase currentPhase) const;
    void GenerateEmptyPlayoffStructure(std::vector<Match>& matches, const std::string& tournamentId, MatchPhase initialPhase, int teamCount) const;
};

// Factory para crear estrategias
class MatchStrategyFactory {
public:
    static std::unique_ptr<IMatchStrategy> CreateStrategy();
};

} // namespace domain

#endif // TOURNAMENTS_IMATCHSTRATEGY_HPP