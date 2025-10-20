#ifndef RESTAPI_TEAMREPOSITORY_HPP
#define RESTAPI_TEAMREPOSITORY_HPP

#include <string>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include "persistence/repository/IRepository.hpp"
#include "domain/Team.hpp"
#include "persistence/configuration/IDbConnectionProvider.hpp"
#include "persistence/configuration/PostgresConnection.hpp"

class TeamRepository : public IRepository<domain::Team, std::string> {
    std::shared_ptr<IDbConnectionProvider> connectionProvider;
public:
    explicit TeamRepository(std::shared_ptr<IDbConnectionProvider> connectionProvider) : connectionProvider(std::move(connectionProvider)) {}

    std::vector<std::shared_ptr<domain::Team>> ReadAll() override {
        std::vector<std::shared_ptr<domain::Team>> teams;
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

        try {
            pqxx::work tx(*(connection->connection));
            pqxx::result result{tx.exec("select id, document->>'name' as name from teams")};
            tx.commit();

            for(auto row : result){
                teams.push_back(std::make_shared<domain::Team>(domain::Team{row["id"].as<std::string>(), row["name"].as<std::string>()}));
            }
        } catch (const std::exception& e) { /* handle error */ }
        return teams;
    }

    std::shared_ptr<domain::Team> ReadById(std::string id) override {
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

        try {
            pqxx::work tx(*(connection->connection));
            pqxx::result result{tx.exec_params("SELECT id, document->>'name' as name FROM teams WHERE id = $1", id)};
            tx.commit();

            if (result.empty()) return nullptr;

            auto row = result[0];
            return std::make_shared<domain::Team>(domain::Team{row["id"].as<std::string>(), row["name"].as<std::string>()});
        } catch (const std::exception& e) { return nullptr; }
    }

    std::optional<std::string> Create(const domain::Team &entity) override {
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
        nlohmann::json teamBody = entity;

        try {
            pqxx::work tx(*(connection->connection));
            pqxx::result result = tx.exec_params("INSERT INTO teams (document) VALUES ($1::jsonb) RETURNING id;", teamBody.dump());
            tx.commit();
            return result[0]["id"].as<std::string>();
        } catch (const pqxx::unique_violation& e) {
            return std::nullopt;
        } catch (const std::exception& e) {
            return std::nullopt;
        }
    }

    std::string Update(const domain::Team &entity) override {
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
        nlohmann::json teamBody = entity;

        try {
            pqxx::work tx(*(connection->connection));
            tx.exec_params("UPDATE teams SET document = $1::jsonb WHERE id = $2", teamBody.dump(), entity.Id);
            tx.commit();
            return entity.Id;
        } catch (const std::exception& e) { return ""; }
    }

    void Delete(std::string id) override {
        auto pooled = connectionProvider->Connection();
        auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

        try {
            pqxx::work tx(*(connection->connection));
            tx.exec_params("DELETE FROM teams WHERE id = $1", id);
            tx.commit();
        } catch (const std::exception& e) { /* handle error */ }
    }
};

#endif //RESTAPI_TEAMREPOSITORY_HPP