#ifndef DOMAIN_GROUP_HPP
#define DOMAIN_GROUP_HPP

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "domain/Team.hpp"

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
    };

    // --- Funciones de Serialización JSON ---

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