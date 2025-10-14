#include "persistence/repository/GroupRepository.hpp"
#include "persistence/configuration/PostgresConnection.hpp"
#include "domain/Group.hpp" // Incluimos la definición completa de Group aquí
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
    return nullptr; // Lógica pendiente
}

std::string GroupRepository::Update(const domain::Group & entity) {
    return ""; // Lógica pendiente
}

void GroupRepository::Delete(std::string id) {
    // Lógica pendiente
}

std::vector<std::shared_ptr<domain::Group>> GroupRepository::ReadAll() {
    return {}; // Lógica pendiente
}