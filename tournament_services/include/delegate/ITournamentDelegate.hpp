#ifndef RESTAPI_ITOURNAMENTDELEGATE_HPP
#define RESTAPI_ITOURNAMENTDELEGATE_HPP

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <expected>
#include "domain/Tournament.hpp"

class ITournamentDelegate {
public:
    enum class SaveError { Conflict, NotFound, Unknown };
    
    virtual ~ITournamentDelegate() = default;
    
    virtual std::expected<std::string, SaveError> CreateTournament(std::shared_ptr<domain::Tournament> tournament) = 0;
    virtual std::shared_ptr<domain::Tournament> GetTournament(std::string_view id) = 0;
    virtual std::vector<std::shared_ptr<domain::Tournament>> GetAllTournaments() = 0;
    virtual std::expected<void, SaveError> UpdateTournament(std::string_view id, const domain::Tournament& tournament) = 0;
    virtual std::expected<void, SaveError> DeleteTournament(std::string_view id) = 0;
};

#endif //RESTAPI_ITOURNAMENTDELEGATE_HPP