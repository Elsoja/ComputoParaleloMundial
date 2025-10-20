#include "persistence/repository/TournamentRepository.hpp"
#include "persistence/configuration/PostgresConnection.hpp"
#include "domain/Tournament.hpp"
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

TournamentRepository::TournamentRepository(std::shared_ptr<IDbConnectionProvider> connection) : connectionProvider(std::move(connection)) {}

std::shared_ptr<domain::Tournament> TournamentRepository::ReadById(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        // ✅ CAMBIO: Volvemos a usar exec_params
        const pqxx::result result = tx.exec_params("SELECT id, document FROM tournaments WHERE id = $1", id);
        tx.commit();

        if (result.empty()) {
            return nullptr;
        }
        nlohmann::json rowTournament = nlohmann::json::parse(result.at(0)["document"].c_str());
        auto tournament = std::make_shared<domain::Tournament>();
        from_json(rowTournament, *tournament);
        tournament->Id() = result.at(0)["id"].as<std::string>();
        return tournament;
    } catch (const std::exception& e) {
        return nullptr;
    }
}

std::optional<std::string> TournamentRepository::Create(const domain::Tournament& entity) {
    const nlohmann::json tournamentDoc = entity;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        // ✅ CAMBIO: Volvemos a usar exec_params
        const pqxx::result result = tx.exec_params("INSERT INTO tournaments (document) VALUES ($1::jsonb) RETURNING id;", tournamentDoc.dump());
        tx.commit();
        return result[0]["id"].as<std::string>();
    } catch (const pqxx::unique_violation& e) {
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::string TournamentRepository::Update(const domain::Tournament& entity) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    nlohmann::json tournamentDoc = entity;

    try {
        pqxx::work tx(*(connection->connection));
        // ✅ CAMBIO: Volvemos a usar exec_params
        tx.exec_params("UPDATE tournaments SET document = $1::jsonb WHERE id = $2", tournamentDoc.dump(), entity.Id());
        tx.commit();
        return entity.Id();
    } catch (const std::exception& e) {
        return "";
    }
}

void TournamentRepository::Delete(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        // ✅ CAMBIO: Volvemos a usar exec_params
        tx.exec_params("DELETE FROM tournaments WHERE id = $1", id);
        tx.commit();
    } catch (const std::exception& e) {
        // Manejar error si es necesario
    }
}

std::vector<std::shared_ptr<domain::Tournament>> TournamentRepository::ReadAll() {
    std::vector<std::shared_ptr<domain::Tournament>> tournaments;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        // Este no necesita parámetros, así que `exec` es correcto aquí
        const pqxx::result result{tx.exec_params("SELECT id, document FROM tournaments")};
        tx.commit();

        for (auto row : result) {
            nlohmann::json rowTournament = nlohmann::json::parse(row["document"].c_str());
            auto tournament = std::make_shared<domain::Tournament>();
            from_json(rowTournament, *tournament);
            tournament->Id() = row["id"].as<std::string>();
            tournaments.push_back(tournament);
        }
    } catch (const std::exception& e) {
        // Manejar error
    }
    return tournaments;
}