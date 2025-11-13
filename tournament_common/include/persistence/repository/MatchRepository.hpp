#ifndef TOURNAMENTS_MATCHREPOSITORY_HPP
#define TOURNAMENTS_MATCHREPOSITORY_HPP

#include "persistence/repository/IMatchRepository.hpp"
#include "persistence/configuration/IDbConnectionProvider.hpp"


namespace repository {
    class MatchRepository : public IMatchRepository {
        std::shared_ptr<IDbConnectionProvider> connectionProvider;
    public:
        explicit MatchRepository(std::shared_ptr<IDbConnectionProvider> provider);


        std::optional<std::string> Create(const domain::Match& entity) override;
        std::shared_ptr<domain::Match> ReadById(std::string id) override;
        std::string Update(const domain::Match& entity) override;
        void Delete(std::string id) override;
        std::vector<std::shared_ptr<domain::Match>> ReadAll() override;

        // Métodos de IMatchRepository
        std::vector<std::shared_ptr<domain::Match>> FindByTournamentId(std::string tournamentId) override;
        std::vector<std::shared_ptr<domain::Match>> FindByTournamentIdAndPhase(std::string tournamentId, domain::MatchPhase phase) override;
        std::vector<std::shared_ptr<domain::Match>> FindByGroupId(std::string groupId) override;
        std::vector<std::shared_ptr<domain::Match>> FindByTeamId(std::string teamId) override;
        bool IsGroupStageComplete(std::string tournamentId) override;
        domain::Match Save(const domain::Match& match) override;
    };
} // namespace repository

#endif //TOURNAMENTS_MATCHREPOSITORY_HPP