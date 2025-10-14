// Update the path below if the header is located elsewhere, e.g.:
#include "delegate/TeamDelegate.hpp"
#include <utility>

TeamDelegate::TeamDelegate(std::shared_ptr<IRepository<domain::Team, std::string>> repository) 
    : teamRepository(std::move(repository)) {}

std::shared_ptr<domain::Team> TeamDelegate::GetTeam(std::string_view id) {
    return teamRepository->ReadById(std::string(id));
}

std::vector<std::shared_ptr<domain::Team>> TeamDelegate::GetAllTeams() {
    return teamRepository->ReadAll();
}

// CAMBIO: La lógica ahora devuelve directamente el std::optional del repositorio
std::optional<std::string> TeamDelegate::SaveTeam(const domain::Team& team) {
    return teamRepository->Create(team);
}