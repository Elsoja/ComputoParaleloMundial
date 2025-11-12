#ifndef DOMAIN_GROUP_HPP
#define DOMAIN_GROUP_HPP

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "Team.hpp" 

namespace domain {
    class Group {
        std::string id;
        std::string name;
        int maxTeams = 4;
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

        [[nodiscard]] bool IsFull() const {
            return teams.size() >= static_cast<size_t>(maxTeams);
        }

        int MaxTeams() const { return maxTeams; }
        void SetMaxTeams(int max) { maxTeams = max; }
            
        // Dos grupos son iguales si su Id, Name, TournamentId y equipos son iguales.
        bool operator==(const Group& other) const {
            return id == other.id &&
                   name == other.name &&
                   tournamentId == other.tournamentId &&
                   teams == other.teams; // Asume que Team tiene operator==
        }
    };

     // --- Funciones de Serialización JSON ---
    // (Asegúrate de que estas funciones estén completas y correctas)
    inline void to_json(nlohmann::json& j, const Group& g) {
        j = nlohmann::json{
            {"id", g.Id()}, 
            {"name", g.Name()},
            {"tournamentId", g.TournamentId()},
            {"teams", g.Teams()}
        };
    }
    
    inline void from_json(const nlohmann::json& j, Group& g) {
        g.Id() = j.value("id", "");
        g.Name() = j.value("name", "");
        g.TournamentId() = j.value("tournamentId", "");
        if (j.contains("teams")) {
            j.at("teams").get_to(g.Teams());
        }
    }
    
    inline void to_json(nlohmann::json& j, const std::shared_ptr<domain::Group>& g) {
        if (g) {
            to_json(j, *g);
        } else {
            j = nullptr;
        }
    }

}
#endif //DOMAIN_GROUP_HPP