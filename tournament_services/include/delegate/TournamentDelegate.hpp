#ifndef RESTAPI_TOURNAMENTDELEGATE_HPP
#define RESTAPI_TOURNAMENTDELEGATE_HPP

#include "delegate/ITournamentDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "cms/QueueMessageProducer.hpp"
#include <memory>

// Forward declaration para evitar include circular si es necesario
namespace domain {
    class Tournament;
}

class TournamentDelegate : public ITournamentDelegate {
    std::shared_ptr<IRepository<domain::Tournament, std::string>> tournamentRepository;
    std::shared_ptr<QueueMessageProducer> producer;

public:
    // El constructor que ya tenías
    explicit TournamentDelegate(std::shared_ptr<IRepository<domain::Tournament, std::string>> repository, std::shared_ptr<QueueMessageProducer> producer);

    ~TournamentDelegate() override = default;

    // CAMBIO: La firma ahora es consistente y devuelve std::expected
    std::expected<std::string, SaveError> CreateTournament(std::shared_ptr<domain::Tournament> tournament) override;

    std::vector<std::shared_ptr<domain::Tournament>> ReadAll() override;
};

#endif //RESTAPI_TOURNAMENTDELEGATE_HPP