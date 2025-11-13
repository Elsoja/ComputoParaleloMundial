#ifndef RESTAPI_ITEAMDELEGATE_HPP
#define RESTAPI_ITEAMDELEGATE_HPP

#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <expected>
#include "domain/Team.hpp"

class ITeamDelegate {
public:
    // Se añade 'NotFound' para errores al actualizar/borrar
    enum class SaveError { Conflict, NotFound, Unknown }; 

    virtual ~ITeamDelegate() = default;

    // Métodos existentes
    virtual std::expected<std::string, SaveError> SaveTeam(const domain::Team& team) = 0;
    virtual std::shared_ptr<domain::Team> GetTeam(std::string_view id) = 0;
    virtual std::vector<std::shared_ptr<domain::Team>> GetAllTeams() = 0;
    virtual std::expected<void, SaveError> UpdateTeam(std::string_view id, const domain::Team& team) = 0;
    virtual std::expected<void, SaveError> DeleteTeam(std::string_view id) = 0;
};

#endif //RESTAPI_ITEAMDELEGATE_HPP 