#ifndef SERVICE_GROUP_DELEGATE_HPP
#define SERVICE_GROUP_DELEGATE_HPP

#include <string>
#include <string_view>
#include <memory>
#include <expected>
#include <utility> // Para std::move

#include "IGroupDelegate.hpp"
// CAMBIO: Incluir los repositorios necesarios para la implementación
#include "persistence/repository/TournamentRepository.hpp"
#include "persistence/repository/GroupRepository.hpp"
#include "persistence/repository/TeamRepository.hpp"

class GroupDelegate : public IGroupDelegate{
    std::shared_ptr<TournamentRepository> tournamentRepository;
    std::shared_ptr<GroupRepository> groupRepository;
    std::shared_ptr<TeamRepository> teamRepository;

public:
    GroupDelegate(const std::shared_ptr<TournamentRepository>& tournamentRepository, const std::shared_ptr<GroupRepository>& groupRepository, const std::shared_ptr<TeamRepository>& teamRepository);
    std::expected<std::string, std::string> CreateGroup(const std::string_view& tournamentId, const domain::Group& group) override;
    std::expected<std::vector<domain::Group>, std::string> GetGroups(const std::string_view& tournamentId) override;
    std::expected<domain::Group, std::string> GetGroup(const std::string_view& tournamentId, const std::string_view& groupId) override;
    std::expected<void, std::string> UpdateGroup(const std::string_view& tournamentId, const domain::Group& group) override;
    std::expected<void, std::string> RemoveGroup(const std::string_view& tournamentId, const std::string_view& groupId) override;
};

// --- Implementaciones ---

inline GroupDelegate::GroupDelegate(const std::shared_ptr<TournamentRepository>& tournamentRepository, const std::shared_ptr<GroupRepository>& groupRepository, const std::shared_ptr<TeamRepository>& teamRepository)
    : tournamentRepository(std::move(tournamentRepository)), groupRepository(std::move(groupRepository)), teamRepository(std::move(teamRepository)){}

inline std::expected<std::string, std::string> GroupDelegate::CreateGroup(const std::string_view& tournamentId, const domain::Group& group) {
    auto tournament = tournamentRepository->ReadById(tournamentId.data());
    if (tournament == nullptr) {
        return std::unexpected("Tournament doesn't exist");
    }

    domain::Group g = group; // Copiamos el grupo para poder modificarlo
    g.TournamentId() = tournament->Id();

    if (!g.Teams().empty()) {
        for (auto& t : g.Teams()) {
            auto team = teamRepository->ReadById(t.Id);
            if (team == nullptr) {
                return std::unexpected("Team doesn't exist");
            }
        }
    }

    // CAMBIO: Lógica para manejar la respuesta del repositorio
    // 1. Llamamos al repositorio, que devuelve std::optional<string>
    auto idOptional = groupRepository->Create(g);

    // 2. Verificamos el resultado del optional
    if (idOptional) {
        // Si tiene valor, la operación fue un éxito. Devolvemos el valor.
        return idOptional.value();
    } else {
        // Si está vacío, hubo un conflicto. Devolvemos un error.
        return std::unexpected("Group could not be created, possibly a duplicate name.");
    }
}

inline std::expected<std::vector<domain::Group>, std::string> GroupDelegate::GetGroups(const std::string_view& tournamentId) {
    std::vector<domain::Group> groups;
    return groups;
}
inline std::expected<domain::Group, std::string> GroupDelegate::GetGroup(const std::string_view& tournamentId, const std::string_view& groupId) {
    // Es necesario inicializar el objeto Group ya que no tiene constructor por defecto explícito
    domain::Group group("demo group");
    return group;
}
inline std::expected<void, std::string> GroupDelegate::UpdateGroup(const std::string_view& tournamentId, const domain::Group& group) {
    return std::unexpected("Not implemented");
}
inline std::expected<void, std::string> GroupDelegate::RemoveGroup(const std::string_view& tournamentId, const std::string_view& groupId) {
    return std::unexpected("Not implemented");
}

#endif /* SERVICE_GROUP_DELEGATE_HPP */