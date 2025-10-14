#ifndef RESTAPI_ITOURNAMENTDELEGATE_HPP
#define RESTAPI_ITOURNAMENTDELEGATE_HPP

#include <memory>
#include <string>
#include <vector>
#include <expected>
#include "domain/Tournament.hpp"

class ITournamentDelegate {
public:
    enum class SaveError { Conflict, Unknown };
    
    virtual ~ITournamentDelegate() = default;
    
    // CAMBIO: La firma ahora usa CreateTournament y devuelve std::expected
    virtual std::expected<std::string, SaveError> CreateTournament(std::shared_ptr<domain::Tournament> tournament) = 0;
    
    virtual std::vector<std::shared_ptr<domain::Tournament>> ReadAll() = 0;
    // Añade aquí las otras firmas de métodos que necesites (ej. GetTournament)
};

#endif //RESTAPI_ITOURNAMENTDELEGATE_HPP