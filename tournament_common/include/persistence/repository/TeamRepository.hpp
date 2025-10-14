//
// Created by tomas on 8/24/25.
//

#ifndef RESTAPI_TEAMREPOSITORY_HPP
#define RESTAPI_TEAMREPOSITORY_HPP
#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <optional> // CAMBIO: Incluir para std::optional

#include "persistence/configuration/IDbConnectionProvider.hpp"
#include "persistence/configuration/PostgresConnection.hpp"
#include "IRepository.hpp"
#include "domain/Team.hpp"
#include "domain/Utilities.hpp"


// CAMBIO: La interfaz ahora usa std::string
class TeamRepository : public IRepository<domain::Team, std::string> {
    std::shared_ptr<IDbConnectionProvider> connectionProvider;
public:

    explicit TeamRepository(std::shared_ptr<IDbConnectionProvider> connectionProvider) : connectionProvider(std::move(connectionProvider)){}

    std::vector<std::shared_ptr<domain::Team>> ReadAll() override {
        std::vector<std::shared_ptr<domain::Team>> teams;
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
        
        pqxx::work tx(*(connection->connection));
        pqxx::result result{tx.exec("select id, document->>'name' as name from teams")};
        tx.commit();

        for(auto row : result){
            teams.push_back(std::make_shared<domain::Team>(domain::Team{row["id"].as<std::string>(), row["name"].as<std::string>()}));
        }
        return teams;
    }

    std::shared_ptr<domain::Team> ReadById(std::string id) override {
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
        
        pqxx::work tx(*(connection->connection));
        // CAMBIO: Implementación real en lugar de un stub
        pqxx::result result{tx.exec_params("SELECT id, document->>'name' as name FROM teams WHERE id = $1", id)};
        tx.commit();
        
        if (result.empty()) {
            return nullptr;
        }
        
        auto row = result[0];
        return std::make_shared<domain::Team>(domain::Team{row["id"].as<std::string>(), row["name"].as<std::string>()});
    }

    // CAMBIO: El método completo con manejo de errores
    std::optional<std::string> Create(const domain::Team &entity) override {
        nlohmann::json teamBody = entity;
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

        try {
            pqxx::work tx(*(connection->connection));
            // Asumo que tu función preparada 'insert_team' hace esto:
            pqxx::result result = tx.exec_params("INSERT INTO teams (document) VALUES ($1::jsonb) RETURNING id;", teamBody.dump());
            tx.commit();
            return result[0]["id"].as<std::string>();
        } catch (const pqxx::unique_violation& e) {
            // Si el equipo ya existe, la BD lanza este error.
            // Devolvemos un optional vacío para señalar el conflicto.
            return std::nullopt;
        } catch (const std::exception& e) {
            // Manejar otros errores de BD
            return std::nullopt;
        }
    }

    std::string Update(const domain::Team &entity) override {
        return "newID";
    }


    void Delete(std::string id) override {
        
    }
};


#endif //RESTAPI_TEAMREPOSITORY_HPP