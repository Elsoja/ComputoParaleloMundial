#ifndef SERVICE_GROUP_DELEGATE_HPP
#define SERVICE_GROUP_DELEGATE_HPP

#include <string>
#include <string_view>
#include <memory>
#include <expected>
#include <utility>
#include <vector>

#include "IGroupDelegate.hpp"
#include "persistence/repository/TournamentRepository.hpp"
#include "persistence/repository/GroupRepository.hpp"
#include "persistence/repository/TeamRepository.hpp"
#include "domain/Group.hpp"
#include "domain/Tournament.hpp" 
#include "domain/Team.hpp"

class GroupDelegate : public IGroupDelegate{
    std::shared_ptr<IRepository<domain::Tournament, std::string>> tournamentRepository;
    std::shared_ptr<IRepository<domain::Group, std::string>> groupRepository;
    std::shared_ptr<IRepository<domain::Team, std::string>> teamRepository;

public:
   GroupDelegate(
        const std::shared_ptr<IRepository<domain::Tournament, std::string>>& tournamentRepo,
        const std::shared_ptr<IRepository<domain::Group, std::string>>& groupRepo,
        const std::shared_ptr<IRepository<domain::Team, std::string>>& teamRepo);

    std::expected<std::string, std::string> CreateGroup(const std::string_view& tournamentId, const domain::Group& group) override;
    std::expected<std::vector<domain::Group>, std::string> GetGroups(const std::string_view& tournamentId) override;
    std::expected<domain::Group, std::string> GetGroup(const std::string_view& tournamentId, const std::string_view& groupId) override;
    std::expected<void, std::string> UpdateGroup(const std::string_view& tournamentId, const domain::Group& group) override;
    std::expected<void, std::string> RemoveGroup(const std::string_view& tournamentId, const std::string_view& groupId) override;
};

// --- Implementaciones ---

inline GroupDelegate::GroupDelegate(
    const std::shared_ptr<IRepository<domain::Tournament, std::string>>& tournamentRepo,
    const std::shared_ptr<IRepository<domain::Group, std::string>>& groupRepo,
    const std::shared_ptr<IRepository<domain::Team, std::string>>& teamRepo)
    : tournamentRepository(tournamentRepo), groupRepository(groupRepo), teamRepository(teamRepo) {}
    
inline std::expected<std::string, std::string> GroupDelegate::CreateGroup(const std::string_view& tournamentId, const domain::Group& group) {
    // **Lógica de negocio: Verificar que el torneo exista**
    auto tournament = tournamentRepository->ReadById(tournamentId.data());
    if (tournament == nullptr) {
        return std::unexpected("Tournament not found");
    }

    domain::Group g = group;
    g.TournamentId() = tournament->Id();

    // Lógica opcional: validar que los equipos existan si se proporcionan
    if (!g.Teams().empty()) {
        for (auto& t : g.Teams()) {
            // ✅ CAMBIO: Usar t.Id() con paréntesis
            if (teamRepository->ReadById(t.Id()) == nullptr) {
                // ✅ CAMBIO: Usar t.Id() con paréntesis
                return std::unexpected("Team with id " + t.Id() + " not found");
            }
        }
    }

    auto idOptional = groupRepository->Create(g);

    if (idOptional) {
        return idOptional.value();
    } else {
        return std::unexpected("Group could not be created, possibly a duplicate name.");
    }
}

inline std::expected<std::vector<domain::Group>, std::string> GroupDelegate::GetGroups(const std::string_view& tournamentId) {
    auto allGroupsPtrs = groupRepository->ReadAll();
    std::vector<domain::Group> tournamentGroups;

    for (const auto& groupPtr : allGroupsPtrs) {
        if (groupPtr != nullptr && groupPtr->TournamentId() == tournamentId) {
            tournamentGroups.push_back(*groupPtr);
        }
    }
    return tournamentGroups;
}

inline std::expected<domain::Group, std::string> GroupDelegate::GetGroup(const std::string_view& tournamentId, const std::string_view& groupId) {
    auto groupPtr = groupRepository->ReadById(std::string(groupId));
    
    if (groupPtr == nullptr || groupPtr->TournamentId() != tournamentId) {
        return std::unexpected("Group not found in this tournament");
    }
    
    return *groupPtr;
}

inline std::expected<void, std::string> GroupDelegate::UpdateGroup(const std::string_view& tournamentId, const domain::Group& group) {
    auto existingGroup = groupRepository->ReadById(group.Id());
    
    if (existingGroup == nullptr || existingGroup->TournamentId() != tournamentId) {
        return std::unexpected("Group not found in this tournament");
    }

    groupRepository->Update(group);
    return {};
}

inline std::expected<void, std::string> GroupDelegate::RemoveGroup(const std::string_view& tournamentId, const std::string_view& groupId) {
    auto existingGroup = groupRepository->ReadById(std::string(groupId));
    
    if (existingGroup == nullptr || existingGroup->TournamentId() != tournamentId) {
        return std::unexpected("Group not found in this tournament");
    }

    groupRepository->Delete(std::string(groupId));
    return {};
}

#endif /* SERVICE_GROUP_DELEGATE_HPP */