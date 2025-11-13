#include "delegate/TeamDelegate.hpp"
#include <utility>
#include <expected>

TeamDelegate::TeamDelegate(std::shared_ptr<IRepository<domain::Team, std::string>> repository) : teamRepository(std::move(repository)) {}

std::expected<std::string, ITeamDelegate::SaveError> TeamDelegate::SaveTeam(const domain::Team& team) {
    // 1. Llamamos al repositorio, que devuelve std::optional<string>
    auto idOptional = teamRepository->Create(team);

    // 2. Verificamos el resultado del optional y lo convertimos a expected
    if (idOptional) {
        // Si tiene valor, la operación fue un éxito. Devolvemos el valor.
        return idOptional.value();
    } else {
        // Si está vacío, hubo un conflicto. Devolvemos un error.
        return std::unexpected(ITeamDelegate::SaveError::Conflict);
    }
}

std::shared_ptr<domain::Team> TeamDelegate::GetTeam(std::string_view id) {
    return teamRepository->ReadById(std::string(id));
}

std::vector<std::shared_ptr<domain::Team>> TeamDelegate::GetAllTeams() {
    return teamRepository->ReadAll();
}

// La implementación de UpdateTeam
std::expected<void, ITeamDelegate::SaveError> TeamDelegate::UpdateTeam(std::string_view id, const domain::Team& team) {
    if (teamRepository->ReadById(std::string(id)) == nullptr) {
        return std::unexpected(ITeamDelegate::SaveError::NotFound);
    }

    teamRepository->Update(team);
    return {};
}

// La implementación de DeleteTeam
std::expected<void, ITeamDelegate::SaveError> TeamDelegate::DeleteTeam(std::string_view id) {
    if (teamRepository->ReadById(std::string(id)) == nullptr) {
        return std::unexpected(ITeamDelegate::SaveError::NotFound);
    }

    teamRepository->Delete(std::string(id));
    return {};
}