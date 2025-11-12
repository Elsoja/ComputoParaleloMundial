#ifndef DOMAIN_MATCH_HPP
#define DOMAIN_MATCH_HPP

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <nlohmann/json.hpp>
#include "domain/Team.hpp"

namespace domain {
    
    enum class MatchPhase {
        GROUP_STAGE, ROUND_OF_16, QUARTERFINALS, SEMIFINALS, FINALS
    };
    enum class MatchStatus {
        PENDING, COMPLETED
    };

    class Match {
        std::string id;
        std::string tournamentId;
        std::string groupId; // groupId es string, como los demás IDs
        MatchPhase phase;
        int matchNumber = 1;
        
        std::optional<std::string> team1Id;
        std::optional<std::string> team2Id;
        std::optional<int> team1Score;
        std::optional<int> team2Score;
        
        MatchStatus status = MatchStatus::PENDING;
        std::optional<std::string> nextMatchId;

    public:
        Match() = default;
        Match(const std::string& tournId, MatchPhase ph, int num)
            : tournamentId(tournId), phase(ph), matchNumber(num) {}

        // --- Getters y Setters (Corregidos) ---
        // Getters (const) para leer
        [[nodiscard]] std::string Id() const { return id; }
        [[nodiscard]] std::string TournamentId() const { return tournamentId; }
        [[nodiscard]] std::string GroupId() const { return groupId; }
        [[nodiscard]] MatchPhase Phase() const { return phase; }
        [[nodiscard]] int MatchNumber() const { return matchNumber; }
        [[nodiscard]] const std::optional<std::string>& Team1Id() const { return team1Id; }
        [[nodiscard]] const std::optional<std::string>& Team2Id() const { return team2Id; }
        [[nodiscard]] const std::optional<int>& Team1Score() const { return team1Score; }
        [[nodiscard]] const std::optional<int>& Team2Score() const { return team2Score; }
        [[nodiscard]] MatchStatus Status() const { return status; }
        [[nodiscard]] const std::optional<std::string>& NextMatchId() const { return nextMatchId; }

        // Setters (no-const) para modificar
        std::string& Id() { return id; }
        std::string& TournamentId() { return tournamentId; }
        std::string& GroupId() { return groupId; }
        void SetGroupId(const std::string& gId) { groupId = gId; }
        MatchPhase& Phase() { return phase; }
        int& MatchNumber() { return matchNumber; }
        std::optional<std::string>& Team1Id() { return team1Id; }
        void SetTeam1(const std::string& id) { team1Id = id; }
        std::optional<std::string>& Team2Id() { return team2Id; }
        void SetTeam2(const std::string& id) { team2Id = id; }
        std::optional<int>& Team1Score() { return team1Score; }
        std::optional<int>& Team2Score() { return team2Score; }
        void SetScore(int score1, int score2) {
            team1Score = score1;
            team2Score = score2;
            status = MatchStatus::COMPLETED;
        }
        MatchStatus& Status() { return status; }
        std::optional<std::string>& NextMatchId() { return nextMatchId; }
        void SetNextMatchId(const std::string& nextId) { nextMatchId = nextId; }

        [[nodiscard]] bool IsComplete() const { return status == MatchStatus::COMPLETED; }
        [[nodiscard]] bool HasBothTeams() const { return team1Id.has_value() && team2Id.has_value(); }

        [[nodiscard]] std::optional<std::string> WinnerId() const {
            if (!IsComplete() || !team1Score.has_value() || !team2Score.has_value() || *team1Score == *team2Score) {
                return std::nullopt;
            }
            return (*team1Score > *team2Score) ? team1Id : team2Id;
        }

        bool operator==(const Match& other) const { return id == other.id; }
        
        static std::string PhaseToString(MatchPhase phase);
        static std::string StatusToString(MatchStatus status);
        static MatchPhase StringToPhase(const std::string& s);
        static MatchStatus StringToStatus(const std::string& s);
    };

    // --- Funciones de Serialización JSON ---
    NLOHMANN_JSON_SERIALIZE_ENUM(MatchPhase, {
        {MatchPhase::GROUP_STAGE, "GROUP_STAGE"}, {MatchPhase::ROUND_OF_16, "ROUND_OF_16"},
        {MatchPhase::QUARTERFINALS, "QUARTERFINALS"}, {MatchPhase::SEMIFINALS, "SEMIFINALS"},
        {MatchPhase::FINALS, "FINALS"}
    })
    NLOHMANN_JSON_SERIALIZE_ENUM(MatchStatus, {
        {MatchStatus::PENDING, "PENDING"}, {MatchStatus::COMPLETED, "COMPLETED"}
    })

    inline void to_json(nlohmann::json& j, const Match& m) {
        j = nlohmann::json{
            {"id", m.Id()}, {"tournamentId", m.TournamentId()}, {"groupId", m.GroupId()},
            {"phase", m.Phase()}, {"matchNumber", m.MatchNumber()},
            {"team1Id", m.Team1Id()}, {"team2Id", m.Team2Id()},
            {"team1Score", m.Team1Score()}, {"team2Score", m.Team2Score()},
            {"status", m.Status()}, {"nextMatchId", m.NextMatchId()}
        };
    }
    inline void from_json(const nlohmann::json& j, Match& m) {
        m.Id() = j.value("id", "");
        m.TournamentId() = j.value("tournamentId", "");
        m.GroupId() = j.value("groupId", "");
        m.Phase() = j.value("phase", MatchPhase::GROUP_STAGE);
        m.MatchNumber() = j.value("matchNumber", 0);
        
        // ✅ CAMBIO: Manejo correcto de nullopt para tipos opcionales
        m.Team1Id() = j.value("team1Id", std::optional<std::string>{});
        m.Team2Id() = j.value("team2Id", std::optional<std::string>{});
        m.Team1Score() = j.value("team1Score", std::optional<int>{});
        m.Team2Score() = j.value("team2Score", std::optional<int>{});
        m.Status() = j.value("status", MatchStatus::PENDING);
        m.NextMatchId() = j.value("nextMatchId", std::optional<std::string>{});
    }
    inline void to_json(nlohmann::json& j, const std::shared_ptr<domain::Match>& m) {
        j = m ? nlohmann::json(*m) : nullptr;
    }

} // namespace domain
#endif // DOMAIN_MATCH_HPP