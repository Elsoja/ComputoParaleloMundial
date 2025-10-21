#ifndef DOMAIN_TOURNAMENT_HPP
#define DOMAIN_TOURNAMENT_HPP

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

#include "domain/Group.hpp"
#include "domain/Match.hpp"

namespace domain {
    enum class TournamentType {
        ROUND_ROBIN, NFL
    };

    class TournamentFormat {
        int numberOfGroups;
        int maxTeamsPerGroup;
        TournamentType type;
    public:
        TournamentFormat(int numberOfGroups = 1, int maxTeamsPerGroup = 16, TournamentType tournamentType = TournamentType::ROUND_ROBIN) {
            this->numberOfGroups = numberOfGroups;
            this->maxTeamsPerGroup = maxTeamsPerGroup;
            this->type = tournamentType;
        }
        int NumberOfGroups() const { return this->numberOfGroups; }
        int& NumberOfGroups() { return this->numberOfGroups; }
        int MaxTeamsPerGroup() const { return this->maxTeamsPerGroup; }
        int& MaxTeamsPerGroup() { return this->maxTeamsPerGroup; }
        TournamentType Type() const { return this->type; }
        TournamentType& Type() { return this->type; }

        // Operador== para TournamentFormat
        bool operator==(const TournamentFormat& other) const {
            return numberOfGroups == other.numberOfGroups &&
                   maxTeamsPerGroup == other.maxTeamsPerGroup &&
                   type == other.type;
        }
    };

    class Tournament {
        std::string id;
        std::string name;
        TournamentFormat format;
        std::vector<Group> groups;
        std::vector<Match> matches;

    public:
        explicit Tournament(const std::string &name = "", TournamentFormat format = TournamentFormat()) {
            this->name = name;
            this->format = format;
        }
        [[nodiscard]] std::string Id() const { return this->id; }
        std::string& Id() { return this->id; }
        [[nodiscard]] std::string Name() const { return this->name; }
        std::string& Name() { return this->name; }
        [[nodiscard]] const TournamentFormat& Format() const { return this->format; }
        TournamentFormat& Format() { return this->format; }
        std::vector<Group>& Groups() { return this->groups; }
        [[nodiscard]] const std::vector<Match>& Matches() const { return this->matches; }

        bool operator==(const Tournament& other) const {
            return id == other.id && name == other.name && format == other.format;
        }
    };

   // --- Funciones de Serialización JSON ---
    // (Implementaciones de ejemplo para TournamentFormat)
    inline void to_json(nlohmann::json& j, const TournamentFormat& f) { /* ... */ }
    inline void from_json(const nlohmann::json& j, TournamentFormat& f) { /* ... */ }

    inline void to_json(nlohmann::json& j, const Tournament& t) {
        j = nlohmann::json{
            {"id", t.Id()}, 
            {"name", t.Name()},
            {"format", t.Format()}
        };
    }

    inline void from_json(const nlohmann::json& j, Tournament& t) {
        t.Id() = j.value("id", ""); // Usar .value para evitar errores si "id" no está
        t.Name() = j.value("name", "");
        if (j.contains("format"))
            j.at("format").get_to(t.Format());
    }

    inline void to_json(nlohmann::json& j, const std::shared_ptr<domain::Tournament>& t) {
        if (t) {
            to_json(j, *t);
        } else {
            j = nullptr;
        }
    }

}

#endif // DOMAIN_TOURNAMENT_HPP