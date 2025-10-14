#include "persistence/repository/TournamentRepository.hpp"
#include "persistence/configuration/PostgresConnection.hpp"
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>

TournamentRepository::TournamentRepository(std::shared_ptr<IDbConnectionProvider> connection) : connectionProvider(std::move(connection)) {}

std::shared_ptr<domain::Tournament> TournamentRepository::ReadById(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    pqxx::work tx(*(connection->connection));
    const pqxx::result result = tx.exec_params("SELECT id, document FROM tournaments WHERE id = $1", id);
    tx.commit();

    if (result.empty()) {
        return nullptr;
    }
    nlohmann::json rowTournament = nlohmann::json::parse(result.at(0)["document"].c_str());
    auto tournament = std::make_shared<domain::Tournament>();
    from_json(rowTournament, *tournament); // Usamos la función de serialización
    tournament->Id() = result.at(0)["id"].as<std::string>();

    return tournament;
}

// CAMBIO: La implementación ahora coincide con la nueva firma del .hpp
std::optional<std::string> TournamentRepository::Create(const domain::Tournament& entity) {
    const nlohmann::json tournamentDoc = entity; // Esto funcionará gracias a los cambios en Tournament.hpp
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result = tx.exec_params("INSERT INTO tournaments (document) VALUES ($1::jsonb) RETURNING id;", tournamentDoc.dump());
        tx.commit();
        return result[0]["id"].as<std::string>();
    } catch (const pqxx::unique_violation& e) {
        // Si hay conflicto (ej. nombre duplicado), devolvemos un optional vacío
        return std::nullopt;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::string TournamentRepository::Update(const domain::Tournament& entity) {
    return "id";
}

void TournamentRepository::Delete(std::string id) {
    // Lógica para borrar
}

std::vector<std::shared_ptr<domain::Tournament>> TournamentRepository::ReadAll() {
    std::vector<std::shared_ptr<domain::Tournament>> tournaments;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);

    pqxx::work tx(*(connection->connection));
    const pqxx::result result{tx.exec("SELECT id, document FROM tournaments")};
    tx.commit();

    for (auto row : result) {
        nlohmann::json rowTournament = nlohmann::json::parse(row["document"].c_str());
        auto tournament = std::make_shared<domain::Tournament>();
        from_json(rowTournament, *tournament); // Usamos la función de serialización
        tournament->Id() = row["id"].as<std::string>();
        tournaments.push_back(tournament);
    }
    return tournaments;
}