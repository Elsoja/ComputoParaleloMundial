#ifndef RESTAPI_TEAMDELEGATE_HPP
#define RESTAPI_TEAMDELEGATE_HPP

#include "ITeamDelegate.hpp"
#include "persistence/repository/IRepository.hpp"

class TeamDelegate : public ITeamDelegate {
    std::shared_ptr<IRepository<domain::Team, std::string>> teamRepository;
public:
    explicit TeamDelegate(std::shared_ptr<IRepository<domain::Team, std::string>> repository);
    
    // CAMBIO: Añadimos el destructor para resolver la incompatibilidad de excepciones
    ~TeamDelegate() override = default;

    // CAMBIO: El tipo de retorno ahora es std::optional<std::string>
    std::optional<std::string> SaveTeam(const domain::Team& team) override;

    std::shared_ptr<domain::Team> GetTeam(std::string_view id) override;
    std::vector<std::shared_ptr<domain::Team>> GetAllTeams() override;
};

#endif //RESTAPI_TEAMDELEGATE_HPP