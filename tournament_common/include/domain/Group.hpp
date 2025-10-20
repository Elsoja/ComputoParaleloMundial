#ifndef DOMAIN_GROUP_HPP
#define DOMAIN_GROUP_HPP

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "domain/Team.hpp" // Asegúrate de que Team tenga operator== también

namespace domain {
    class Group {
        std::string id;
        std::string name;
        std::string tournamentId;
        std::vector<Team> teams;

    public:
        explicit Group(const std::string_view & name = "", const std::string_view& id = "") : id(id), name(name) {}

        [[nodiscard]] std::string Id() const { return id; }
        std::string& Id() { return id; }

        [[nodiscard]] std::string Name() const { return name; }
        std::string& Name() { return name; }

        [[nodiscard]] std::string TournamentId() const { return tournamentId; }
        std::string& TournamentId() { return tournamentId; }

        [[nodiscard]] const std::vector<Team>& Teams() const { return teams; }
        std::vector<Team>& Teams() { return teams; }

        // Dos grupos son iguales si su Id, Name, TournamentId y equipos son iguales.
        bool operator==(const Group& other) const {
            return id == other.id &&
                   name == other.name &&
                   tournamentId == other.tournamentId &&
                   teams == other.teams; // std::vector ya tiene operator== si Team lo tiene
        }
    };

    // --- Funciones de Serialización JSON ---
    // (Asegúrate de que estas funciones estén completas y correctas)
    inline void to_json(nlohmann::json& j, const Group& g) { /* ... */ }
    inline void from_json(const nlohmann::json& j, Group& g) { /* ... */ }
    inline void to_json(nlohmann::json& j, const std::shared_ptr<domain::Group>& g) { /* ... */ }

}
#endif //DOMAIN_GROUP_HPP