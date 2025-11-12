#ifndef RESTAPI_DOMAIN_TEAM_HPP
#define RESTAPI_DOMAIN_TEAM_HPP

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace domain {
    class Team {      
        private:
            std::string id;
            std::string name;

        public:
            // Constructores
            Team() : id(""), name("") {}
            Team(const std::string& id, const std::string& name) : id(id), name(name) {}

            // Getters
            std::string Id() const { return id; }
            std::string Name() const { return name; }

            // Setters
            void SetId(const std::string& id) { this->id = id; }
            void SetName(const std::string& name) { this->name = name; }

            // Operador de igualdad
            bool operator==(const Team& other) const {
                return id == other.id && name == other.name;  
            }
    };

    // --- Funciones de Serialización JSON ---

    inline void to_json(nlohmann::json& j, const Team& t) {
        j = nlohmann::json{
            {"id", t.Id()},     
            {"name", t.Name()}   
        };
    }

    inline void from_json(const nlohmann::json& j, Team& t) {
        if (j.contains("id")) {
            t.SetId(j.at("id").get<std::string>());
        }
        t.SetName(j.at("name").get<std::string>());
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