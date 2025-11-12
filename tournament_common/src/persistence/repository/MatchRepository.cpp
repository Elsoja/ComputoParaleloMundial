#include "persistence/repository/MatchRepository.hpp"
#include "persistence/configuration/PostgresConnection.hpp" // Incluir para la conexión
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <utility>

// ✅ CAMBIO: Añadir la apertura del namespace
namespace repository {

// Implementación del constructor
MatchRepository::MatchRepository(std::shared_ptr<IDbConnectionProvider> provider) : connectionProvider(std::move(provider)) {}

// Implementaciones de IRepository
std::optional<std::string> MatchRepository::Create(const domain::Match& entity) {
    const nlohmann::json matchDoc = entity; 
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result = tx.exec_params("INSERT INTO matches (document) VALUES ($1::jsonb) RETURNING id;", matchDoc.dump());
        tx.commit();
        return result[0]["id"].as<std::string>();
    } catch (const std::exception& e) { return std::nullopt; }
}

std::shared_ptr<domain::Match> MatchRepository::ReadById(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result = tx.exec_params("SELECT document FROM matches WHERE id = $1", id);
        tx.commit();
        if (result.empty()) return nullptr;

        nlohmann::json matchDoc = nlohmann::json::parse(result[0]["document"].c_str());
        auto match = std::make_shared<domain::Match>();
        from_json(matchDoc, *match);
        match->Id() = id;
        return match;
    } catch (const std::exception& e) { return nullptr; }
}

std::string MatchRepository::Update(const domain::Match & entity) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    nlohmann::json matchDoc = entity;
    try {
        pqxx::work tx(*(connection->connection));
        tx.exec_params("UPDATE matches SET document = $1::jsonb WHERE id = $2", matchDoc.dump(), entity.Id());
        tx.commit();
        return entity.Id();
    } catch (const std::exception& e) { return ""; }
}

void MatchRepository::Delete(std::string id) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        tx.exec_params("DELETE FROM matches WHERE id = $1", id);
        tx.commit();
    } catch (const std::exception& e) { /* Manejar error */ }
}

std::vector<std::shared_ptr<domain::Match>> MatchRepository::ReadAll() {
    std::vector<std::shared_ptr<domain::Match>> matches;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result{tx.exec("SELECT id, document FROM matches")};
        tx.commit();
        for (auto row : result) {
            nlohmann::json matchDoc = nlohmann::json::parse(row["document"].c_str());
            auto match = std::make_shared<domain::Match>();
            from_json(matchDoc, *match);
            match->Id() = row["id"].as<std::string>();
            matches.push_back(match);
        }
    } catch (const std::exception& e) { /* Manejar error */ }
    return matches;
}

// Implementaciones de IMatchRepository
std::vector<std::shared_ptr<domain::Match>> MatchRepository::FindByTournamentId(std::string tournamentId) {
    std::vector<std::shared_ptr<domain::Match>> matches;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        pqxx::result result = tx.exec_params("SELECT id, document FROM matches WHERE document->>'tournamentId' = $1", tournamentId);
        tx.commit();
        for (auto row : result) {
            nlohmann::json matchDoc = nlohmann::json::parse(row["document"].c_str());
            auto match = std::make_shared<domain::Match>();
            from_json(matchDoc, *match);
            match->Id() = row["id"].as<std::string>();
            matches.push_back(match);
        }
    } catch (const std::exception& e) { /* Manejar error */ }
    return matches;
}

std::vector<std::shared_ptr<domain::Match>> MatchRepository::FindByTournamentIdAndPhase(std::string tournamentId, domain::MatchPhase phase) {
    std::vector<std::shared_ptr<domain::Match>> matches;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        pqxx::result result = tx.exec_params(
            "SELECT id, document FROM matches WHERE document->>'tournamentId' = $1 AND document->>'phase' = $2", 
            tournamentId, 
            domain::Match::PhaseToString(phase) // Usa la función estática
        );
        tx.commit();
        for (auto row : result) {
            nlohmann::json matchDoc = nlohmann::json::parse(row["document"].c_str());
            auto match = std::make_shared<domain::Match>();
            from_json(matchDoc, *match);
            match->Id() = row["id"].as<std::string>();
            matches.push_back(match);
        }
    } catch (const std::exception& e) { /* Manejar error */ }
    return matches;
}

std::vector<std::shared_ptr<domain::Match>> MatchRepository::FindByGroupId(std::string groupId) {
    std::vector<std::shared_ptr<domain::Match>> matches;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        pqxx::result result = tx.exec_params(
            "SELECT id, document FROM matches WHERE document->>'groupId' = $1", 
            groupId
        );
        tx.commit();
        for (auto row : result) {
            nlohmann::json matchDoc = nlohmann::json::parse(row["document"].c_str());
            auto match = std::make_shared<domain::Match>();
            from_json(matchDoc, *match);
            match->Id() = row["id"].as<std::string>();
            matches.push_back(match);
        }
    } catch (const std::exception& e) { /* Manejar error */ }
    return matches;
}

std::vector<std::shared_ptr<domain::Match>> MatchRepository::FindByTeamId(std::string teamId) {
    std::vector<std::shared_ptr<domain::Match>> matches;
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        pqxx::result result = tx.exec_params(
            "SELECT id, document FROM matches WHERE document->>'team1Id' = $1 OR document->>'team2Id' = $1", 
            teamId
        );
        tx.commit();
        for (auto row : result) {
            nlohmann::json matchDoc = nlohmann::json::parse(row["document"].c_str());
            auto match = std::make_shared<domain::Match>();
            from_json(matchDoc, *match);
            match->Id() = row["id"].as<std::string>();
            matches.push_back(match);
        }
    } catch (const std::exception& e) { /* Manejar error */ }
    return matches;
}

bool MatchRepository::IsGroupStageComplete(std::string tournamentId) {
    auto pooled = connectionProvider->Connection();
    const auto connection = dynamic_cast<PostgresConnection*>(&*pooled);
    try {
        pqxx::work tx(*(connection->connection));
        const pqxx::result result = tx.exec_params(R"(
            SELECT 
                COUNT(*) as total,
                COUNT(*) FILTER (WHERE (document->>'status') = 'COMPLETED') as completed
            FROM matches
            WHERE (document->>'tournamentId') = $1 AND (document->>'phase') = 'GROUP_STAGE'
        )", tournamentId);
        tx.commit();

        if (result.empty()) return false;
        auto total = result[0]["total"].as<int>(0);
        auto completed = result[0]["completed"].as<int>(0);
        return (total > 0) && (total == completed);
    } catch (const std::exception& e) { return false; }
}

domain::Match MatchRepository::Save(const domain::Match& match) {
    if (match.Id().empty()) {
        auto newId = Create(match);
        if (newId) {
            auto newMatch = ReadById(*newId);
            if (newMatch) return *newMatch;
        }
    } else {
        Update(match);
        return match;
    }
    throw std::runtime_error("No se pudo guardar el partido (MatchRepository::Save)");
}

} // ✅ CAMBIO: Añadir el cierre del namespace

// ✅ CAMBIO: Mover las funciones estáticas de Match.hpp aquí
namespace domain {
    std::string Match::PhaseToString(MatchPhase phase) {
        switch (phase) {
            case MatchPhase::GROUP_STAGE: return "GROUP_STAGE";
            case MatchPhase::ROUND_OF_16: return "ROUND_OF_16";
            case MatchPhase::QUARTERFINALS: return "QUARTERFINALS";
            case MatchPhase::SEMIFINALS: return "SEMIFINALS";
            case MatchPhase::FINALS: return "FINALS";
            default: return "UNKNOWN";
        }
    }
    std::string Match::StatusToString(MatchStatus status) {
        switch (status) {
            case MatchStatus::PENDING: return "PENDING";
            case MatchStatus::COMPLETED: return "COMPLETED";
            default: return "UNKNOWN";
        }
    }
    MatchPhase Match::StringToPhase(const std::string& s) {
        if (s == "ROUND_OF_16") return MatchPhase::ROUND_OF_16;
        if (s == "QUARTERFINALS") return MatchPhase::QUARTERFINALS;
        if (s == "SEMIFINALS") return MatchPhase::SEMIFINALS;
        if (s == "FINALS") return MatchPhase::FINALS;
        return MatchPhase::GROUP_STAGE;
    }
    MatchStatus Match::StringToStatus(const std::string& s) {
        if (s == "COMPLETED") return MatchStatus::COMPLETED;
        return MatchStatus::PENDING;
    }
}