#ifndef RESTAPI_DOMAIN_TEAM_HPP
#define RESTAPI_DOMAIN_TEAM_HPP

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace domain {
    struct Team {
        std::string Id;
        std::string Name;
    };

    // --- Funciones de Serialización JSON ---

    inline void to_json(nlohmann::json& j, const Team& t) {
        j = nlohmann::json{{"id", t.Id}, {"name", t.Name}};
    }

    inline void from_json(const nlohmann::json& j, Team& t) {
        j.at("id").get_to(t.Id);
        j.at("name").get_to(t.Name);
    }

    inline void to_json(nlohmann::json& j, const std::shared_ptr<domain::Team>& t) {
        if (t) {
            to_json(j, *t);
        } else {
            j = nullptr;
        }
    }

} // namespace domain

#endif //RESTAPI_DOMAIN_TEAM_HPP