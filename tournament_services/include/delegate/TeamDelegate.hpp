#ifndef RESTAPI_TEAMDELEGATE_HPP
#define RESTAPI_TEAMDELEGATE_HPP

#include "delegate/ITeamDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "domain/Team.hpp"
#include <memory>
#include <expected>

class TeamDelegate : public ITeamDelegate {
    std::shared_ptr<IRepository<domain::Team, std::string>> teamRepository;

public:
    explicit TeamDelegate(std::shared_ptr<IRepository<domain::Team, std::string>> repository);
    ~TeamDelegate() override = default;

    std::expected<std::string, SaveError> SaveTeam(const domain::Team& team) override;
    std::shared_ptr<domain::Team> GetTeam(std::string_view id) override;
    std::vector<std::shared_ptr<domain::Team>> GetAllTeams() override;
    std::expected<void, SaveError> UpdateTeam(std::string_view id, const domain::Team& team) override;
    std::expected<void, SaveError> DeleteTeam(std::string_view id) override;
};

#endif // RESTAPI_TEAMDELEGATE_HPP