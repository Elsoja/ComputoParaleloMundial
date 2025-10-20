#include "persistence/repository/GroupRepository.hpp"
#include "persistence/configuration/PostgresConnection.hpp"
#include "domain/Group.hpp"
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <utility>

GroupRepository::GroupRepository(std::shared_ptr<IDbConnectionProvider> provider) : connectionProvider(std::move(provider)) {}

std::optional<std::string> GroupRepository::Create(const domain::Group& entity) {
    const nlohmann::json groupDoc = entity; 
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result = tx.exec_params("INSERT INTO groups (document) VALUES ($1::jsonb) RETURNING id;", groupDoc.dump());
        tx.commit();
        return result[0]["id"].as<std::string>();
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::shared_ptr<domain::Group> GroupRepository::ReadById(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result = tx.exec_params("SELECT document FROM groups WHERE id = $1", id);
        tx.commit();

        if (result.empty()) {
            return nullptr;
        }

        nlohmann::json groupDoc = nlohmann::json::parse(result[0]["document"].c_str());
        auto group = std::make_shared<domain::Group>();
        from_json(groupDoc, *group);
        group->Id() = id;
        return group;

    } catch (const std::exception& e) {
        return nullptr;
    }
}

std::string GroupRepository::Update(const domain::Group & entity) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    nlohmann::json groupDoc = entity;

    try {
        pqxx::work tx(*(connection->connection));
        tx.exec_params("UPDATE groups SET document = $1::jsonb WHERE id = $2", groupDoc.dump(), entity.Id());
        tx.commit();
        return entity.Id();
    } catch (const std::exception& e) {
        return "";
    }
}

void GroupRepository::Delete(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        tx.exec_params("DELETE FROM groups WHERE id = $1", id);
        tx.commit();
    } catch (const std::exception& e) {
        // Manejar error si es necesario
    }
}

std::vector<std::shared_ptr<domain::Group>> GroupRepository::ReadAll() {
    std::vector<std::shared_ptr<domain::Group>> groups;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result{tx.exec("SELECT id, document FROM groups")};
        tx.commit();

        for (auto row : result) {
            nlohmann::json groupDoc = nlohmann::json::parse(row["document"].c_str());
            auto group = std::make_shared<domain::Group>();
            from_json(groupDoc, *group);
            group->Id() = row["id"].as<std::string>();
            groups.push_back(group);
        }
    } catch (const std::exception& e) {
        // Manejar error si es necesario
    }
    return groups;
}