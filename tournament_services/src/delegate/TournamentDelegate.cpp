#include "delegate/TournamentDelegate.hpp"
#include <utility>

TournamentDelegate::TournamentDelegate(
    std::shared_ptr<IRepository<domain::Tournament, std::string>> repository, 
    std::shared_ptr<IQueueMessageProducer> producer) 
    : tournamentRepository(std::move(repository)), producer(std::move(producer)) {
}

std::expected<std::string, ITournamentDelegate::SaveError> TournamentDelegate::CreateTournament(std::shared_ptr<domain::Tournament> tournament) {
    auto idOptional = tournamentRepository->Create(*tournament);
    if (idOptional) {
        const std::string& id = idOptional.value();
        producer->SendMessage(id, "tournament.created");
        return id;
    } else {
        return std::unexpected(ITournamentDelegate::SaveError::Conflict);
    }
}

std::shared_ptr<domain::Tournament> TournamentDelegate::GetTournament(std::string_view id) {
    return tournamentRepository->ReadById(std::string(id));
}

std::vector<std::shared_ptr<domain::Tournament>> TournamentDelegate::GetAllTournaments() {
    return tournamentRepository->ReadAll();
}

std::expected<void, ITournamentDelegate::SaveError> TournamentDelegate::UpdateTournament(std::string_view id, const domain::Tournament& tournament) {
    if (tournamentRepository->ReadById(std::string(id)) == nullptr) {
        return std::unexpected(ITournamentDelegate::SaveError::NotFound);
    }
    tournamentRepository->Update(tournament);
    return {};
}

std::expected<void, ITournamentDelegate::SaveError> TournamentDelegate::DeleteTournament(std::string_view id) {
    if (tournamentRepository->ReadById(std::string(id)) == nullptr) {
        return std::unexpected(ITournamentDelegate::SaveError::NotFound);
    }
    tournamentRepository->Delete(std::string(id));
    return {};
}