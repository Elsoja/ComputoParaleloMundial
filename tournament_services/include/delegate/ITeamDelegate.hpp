#ifndef RESTAPI_ITEAMDELEGATE_HPP
#define RESTAPI_ITEAMDELEGATE_HPP

#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <optional> // CAMBIO: Usamos optional en lugar de expected
#include "domain/Team.hpp"

class ITeamDelegate {
public:
    // CAMBIO: Eliminamos el enum SaveError, ya no es necesario
    virtual ~ITeamDelegate() = default;

    // CAMBIO: El tipo de retorno ahora es std::optional<std::string>
    virtual std::optional<std::string> SaveTeam(const domain::Team& team) = 0;
    
    virtual std::shared_ptr<domain::Team> GetTeam(std::string_view id) = 0;
    virtual std::vector<std::shared_ptr<domain::Team>> GetAllTeams() = 0;
};

#endif //RESTAPI_ITEAMDELEGATE_HPPP