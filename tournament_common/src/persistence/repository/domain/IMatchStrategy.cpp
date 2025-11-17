#include "domain/IMatchStrategy.hpp"
#include <algorithm>
#include <map>

namespace domain {

// --- Estructura Auxiliar para Tablas de Posiciones ---
struct TeamStanding {
    std::string teamId;
    int points = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int goalDifference = 0;
    int wins = 0;
    int draws = 0;
    int losses = 0;

    // Operador de comparación para ordenar (mayor primero)
    bool operator<(const TeamStanding& other) const {
        if (points != other.points) return points > other.points;
        if (goalDifference != other.goalDifference) return goalDifference > other.goalDifference;
        return goalsFor > other.goalsFor;
    }
};

// --- Implementación de SingleEliminationStrategy ---

std::vector<Match> SingleEliminationStrategy::GenerateGroupStageMatches(
    const std::vector<Group>& groups, const std::string& tournamentId) {
    
    std::vector<Match> matches;
    int matchNumber = 1;
    for (const auto& group : groups) {
        const auto& teams = group.Teams();
        for (size_t i = 0; i < teams.size(); i++) {
            for (size_t j = i + 1; j < teams.size(); j++) {
                Match match(tournamentId, MatchPhase::GROUP_STAGE, matchNumber++);
                match.SetTeam1(teams[i].Id()); 
                match.SetTeam2(teams[j].Id());
                match.SetGroupId(group.Id());
                matches.push_back(match);
            }
        }
    }
    return matches;
}

std::vector<Match> SingleEliminationStrategy::GeneratePlayoffMatches(
    const std::vector<Group>& groups, const std::string& tournamentId) {
    
    std::vector<Match> matches;
    auto qualifiedTeams = GetQualifiedTeams(groups);
    if (qualifiedTeams.empty()) return matches;

    //  SEPARAR PRIMEROS Y SEGUNDOS LUGARES
    std::vector<std::string> firstPlaces;   // A1, B1, C1, D1, E1, F1, G1, H1
    std::vector<std::string> secondPlaces;  // A2, B2, C2, D2, E2, F2, G2, H2
    
    for (size_t i = 0; i < qualifiedTeams.size(); i += 2) {
        firstPlaces.push_back(qualifiedTeams[i]);
        if (i + 1 < qualifiedTeams.size()) {
            secondPlaces.push_back(qualifiedTeams[i + 1]);
        }
    }

    // EMPAREJAMIENTOS ESPECÍFICOS ESTILO MUNDIAL
    // Solo aplicar si tenemos exactamente 8 grupos (16 equipos)
    MatchPhase initialPhase = DetermineInitialPhase(qualifiedTeams.size());
    int matchNumber = 1;
    
    if (firstPlaces.size() == 8 && secondPlaces.size() == 8) {
        // Lado superior del bracket
        Match match1(tournamentId, initialPhase, matchNumber++);
        match1.SetTeam1(firstPlaces[0]);  // A1
        match1.SetTeam2(secondPlaces[7]); // H2
        matches.push_back(match1);
        
        Match match2(tournamentId, initialPhase, matchNumber++);
        match2.SetTeam1(firstPlaces[1]);  // B1
        match2.SetTeam2(secondPlaces[6]); // G2
        matches.push_back(match2);
        
        Match match3(tournamentId, initialPhase, matchNumber++);
        match3.SetTeam1(firstPlaces[2]);  // C1
        match3.SetTeam2(secondPlaces[5]); // F2
        matches.push_back(match3);
        
        Match match4(tournamentId, initialPhase, matchNumber++);
        match4.SetTeam1(firstPlaces[3]);  // D1
        match4.SetTeam2(secondPlaces[4]); // E2
        matches.push_back(match4);
        
        // Lado inferior del bracket
        Match match5(tournamentId, initialPhase, matchNumber++);
        match5.SetTeam1(firstPlaces[4]);  // E1
        match5.SetTeam2(secondPlaces[3]); // D2
        matches.push_back(match5);
        
        Match match6(tournamentId, initialPhase, matchNumber++);
        match6.SetTeam1(firstPlaces[5]);  // F1
        match6.SetTeam2(secondPlaces[2]); // C2
        matches.push_back(match6);
        
        Match match7(tournamentId, initialPhase, matchNumber++);
        match7.SetTeam1(firstPlaces[6]);  // G1
        match7.SetTeam2(secondPlaces[1]); // B2
        matches.push_back(match7);
        
        Match match8(tournamentId, initialPhase, matchNumber++);
        match8.SetTeam1(firstPlaces[7]);  // H1
        match8.SetTeam2(secondPlaces[0]); // A2
        matches.push_back(match8);
    } else {
        // Fallback: emparejamiento secuencial para otros formatos
        for (size_t i = 0; i < qualifiedTeams.size(); i += 2) {
            if (i + 1 < qualifiedTeams.size()) {
                Match match(tournamentId, initialPhase, matchNumber++);
                match.SetTeam1(qualifiedTeams[i]);
                match.SetTeam2(qualifiedTeams[i + 1]);
                matches.push_back(match);
            }
        }
    }
    
    GenerateEmptyPlayoffStructure(matches, tournamentId, initialPhase, qualifiedTeams.size());
    return matches;
}

std::optional<Match> SingleEliminationStrategy::GenerateNextMatch(
    const Match& completedMatch,
    const std::vector<std::shared_ptr<domain::Match>>& existingMatches 
) {
    if (completedMatch.NextMatchId().has_value()) {
        return std::nullopt;
    }
    if (completedMatch.Phase() == MatchPhase::FINALS) {
        return std::nullopt;
    }

    MatchPhase nextPhase = GetNextPhase(completedMatch.Phase());
    int nextMatchNumber = (completedMatch.MatchNumber() + 1) / 2;
    Match nextMatch(completedMatch.TournamentId(), nextPhase, nextMatchNumber);
    
    if (completedMatch.WinnerId().has_value()) {
        bool isTeam1 = (completedMatch.MatchNumber() % 2) == 1;
        if (isTeam1) {
            nextMatch.SetTeam1(completedMatch.WinnerId().value());
        } else {
            nextMatch.SetTeam2(completedMatch.WinnerId().value());
        }
    }
    return nextMatch;
}

void SingleEliminationStrategy::UpdateMatchWithWinner(
    Match& match, const std::string& winnerId, bool isTeam1) {
    if (isTeam1) {
        match.SetTeam1(winnerId);
    } else {
        match.SetTeam2(winnerId);
    }
}

bool SingleEliminationStrategy::AreGroupsFull(const std::vector<Group>& groups) const {
    if (groups.empty()) return false;
    for (const auto& group : groups) {
        if (!group.IsFull()) {
            return false;
        }
    }
    return true;
}

// --- Métodos Privados ---

std::vector<std::string> SingleEliminationStrategy::GetQualifiedTeams(const std::vector<Group>& groups) const {
    std::vector<std::string> qualifiedTeams;
    
    
    // --- Lógica de ejemplo (la que tenías): ---
    // TODO: Esta lógica necesita ser reemplazada por el cálculo real de la tabla de posiciones.
    for (const auto& group : groups) {
        const auto& teams = group.Teams();
        for (size_t i = 0; i < std::min(size_t(2), teams.size()); i++) {
            qualifiedTeams.push_back(teams[i].Id());
        }
    }
    return qualifiedTeams;
}

MatchPhase SingleEliminationStrategy::DetermineInitialPhase(int teamCount) const {
    if (teamCount <= 2) return MatchPhase::FINALS;
    if (teamCount <= 4) return MatchPhase::SEMIFINALS;
    // ✅ CAMBIO: Corregido error de tipeo MatchMapPhase -> MatchPhase
    if (teamCount <= 8) return MatchPhase::QUARTERFINALS;
    return MatchPhase::ROUND_OF_16;
}

MatchPhase SingleEliminationStrategy::GetNextPhase(MatchPhase currentPhase) const {
    switch (currentPhase) {
        case MatchPhase::ROUND_OF_16: return MatchPhase::QUARTERFINALS;
        case MatchPhase::QUARTERFINALS: return MatchPhase::SEMIFINALS;
        case MatchPhase::SEMIFINALS: return MatchPhase::FINALS;
        default: return MatchPhase::FINALS;
    }
}

void SingleEliminationStrategy::GenerateEmptyPlayoffStructure(
    std::vector<Match>& matches, const std::string& tournamentId,
    MatchPhase initialPhase, int teamCount) const {
    
    MatchPhase currentPhase = initialPhase;
    int currentMatchCount = teamCount / 2; 

    while (currentPhase != MatchPhase::FINALS) {
        currentPhase = GetNextPhase(currentPhase);
        currentMatchCount = currentMatchCount / 2; 

        if (currentMatchCount < 1) break;

        for (int i = 1; i <= currentMatchCount; i++) {
            Match match(tournamentId, currentPhase, i);
            matches.push_back(match);
        }

        if (currentMatchCount == 1) break; 
    }
}

// --- Implementación del Factory ---
std::unique_ptr<IMatchStrategy> MatchStrategyFactory::CreateStrategy() {
    return std::make_unique<SingleEliminationStrategy>();
}

} // namespace domain