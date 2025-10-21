#ifndef RESTAPI_TOURNAMENTDELEGATE_HPP
#define RESTAPI_TOURNAMENTDELEGATE_HPP

#include "delegate/ITournamentDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "cms/IQueueMessageProducer.hpp" // Usar la interfaz
#include <memory>

// Forward declaration
namespace domain {
    class Tournament;
}

class TournamentDelegate : public ITournamentDelegate {
    // Miembros de la clase
    std::shared_ptr<IRepository<domain::Tournament, std::string>> tournamentRepository;
    std::shared_ptr<IQueueMessageProducer> producer; // Usar la interfaz

public:
    // Declaración del constructor
    explicit TournamentDelegate(
        std::shared_ptr<IRepository<domain::Tournament, std::string>> repository, 
        std::shared_ptr<IQueueMessageProducer> producer
    );
    ~TournamentDelegate() override = default;

    // Declaraciones de los métodos del CRUD
    std::expected<std::string, SaveError> CreateTournament(std::shared_ptr<domain::Tournament> tournament) override;
    std::shared_ptr<domain::Tournament> GetTournament(std::string_view id) override;
    std::vector<std::shared_ptr<domain::Tournament>> GetAllTournaments() override;
    std::expected<void, SaveError> UpdateTournament(std::string_view id, const domain::Tournament& tournament) override;
    std::expected<void, SaveError> DeleteTournament(std::string_view id) override;
};

#endif //RESTAPI_TOURNAMENTDELEGATE_HPP