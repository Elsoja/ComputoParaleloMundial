#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "domain/IMatchStrategy.hpp" // Incluimos la estrategia a probar
#include "domain/Group.hpp"
#include "domain/Team.hpp"
#include "domain/Match.hpp"

// Usamos 'using' para simplificar
using namespace domain;

// --- Prueba para el Escenario 1: Creación de Partidos de Fase de Grupos ---
TEST(IMatchStrategyTest, GroupStageGeneration) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "tourn-1";
    
    // 1. Setup: Crear un grupo con 4 equipos
    Group groupA("Grupo A");
    groupA.SetMaxTeams(4); // Asegurarnos de que el límite sea 4
    groupA.Teams().push_back(Team{"t1", "Equipo 1"});
    groupA.Teams().push_back(Team{"t2", "Equipo 2"});
    groupA.Teams().push_back(Team{"t3", "Equipo 3"});
    groupA.Teams().push_back(Team{"t4", "Equipo 4"});
    
    std::vector<Group> groups = {groupA};

    // 2. Action: Generar los partidos
    auto matches = strategy.GenerateGroupStageMatches(groups, tournamentId);

    // 3. Assert: Verificar que se crearon 6 partidos (4 equipos = 4*(4-1) = 6)
    ASSERT_EQ(matches.size(), 6);
    
    // Verificar que todos sean de fase de grupos
    for (const auto& match : matches) {
        ASSERT_EQ(match.TournamentId(), tournamentId);
        ASSERT_EQ(match.Phase(), MatchPhase::GROUP_STAGE);
    }
    
    // Verificar los pares (opcional pero recomendado)
    ASSERT_EQ(matches[0].Team1Id().value(), "t1");
    ASSERT_EQ(matches[0].Team2Id().value(), "t2");
    // ... (más aserciones para los otros 5 partidos)
}

// --- Prueba para el Escenario 2: No hacer nada si está incompleto ---
TEST(IMatchStrategyTest, AreGroupsFull_FalseWhenIncomplete) {
    SingleEliminationStrategy strategy;
    
    // 1. Setup: Un grupo con solo 3 de 4 equipos
    Group groupA("Grupo A");
    groupA.SetMaxTeams(4);
    groupA.Teams().push_back(Team{"t1", "Equipo 1"});
    groupA.Teams().push_back(Team{"t2", "Equipo 2"});
    groupA.Teams().push_back(Team{"t3", "Equipo 3"});
    
    std::vector<Group> groups = {groupA};

    // 2. Action & Assert
    ASSERT_FALSE(strategy.AreGroupsFull(groups));
}

TEST(IMatchStrategyTest, AreGroupsFull_TrueWhenFull) {
    SingleEliminationStrategy strategy;
    
    // 1. Setup: Un grupo con 4 de 4 equipos
    Group groupA("Grupo A");
    groupA.SetMaxTeams(4);
    groupA.Teams().push_back(Team{"t1", "Equipo 1"});
    groupA.Teams().push_back(Team{"t2", "Equipo 2"});
    groupA.Teams().push_back(Team{"t3", "Equipo 3"});
    groupA.Teams().push_back(Team{"t4", "Equipo 4"});
    
    std::vector<Group> groups = {groupA};

    // 2. Action & Assert
    ASSERT_TRUE(strategy.AreGroupsFull(groups));
}


// --- Prueba para el Escenario 3: Creación de Fase de Playoffs ---
// Esta prueba asume 8 grupos (A-H) con 2 equipos clasificados cada uno (16 total)
TEST(IMatchStrategyTest, PlayoffGeneration_CreatesFullBracket) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "tourn-1";
    
    // 1. Setup: Crear 8 grupos. 
    // (Solo necesitamos los equipos que clasifican, GetQualifiedTeams los extrae)
    std::vector<Group> groups;
    for (int i = 0; i < 8; ++i) {
        Group g(std::string(1, 'A' + i)); // Grupo A, B, C...
        g.SetMaxTeams(4);
        // Añadir 4 equipos (solo los 2 primeros importan para el GetQualifiedTeams actual)
        g.Teams().push_back(Team{std::string(1, 'A' + i) + "1", "Equipo " + std::string(1, 'A' + i) + "1"});
        g.Teams().push_back(Team{std::string(1, 'A' + i) + "2", "Equipo " + std::string(1, 'A' + i) + "2"});
        g.Teams().push_back(Team{"t_extra1", "Equipo Extra 1"});
        g.Teams().push_back(Team{"t_extra2", "Equipo Extra 2"});
        groups.push_back(g);
    }

    // 2. Action: Generar playoffs
    // NOTA: Esto usa la lógica de GetQualifiedTeams (que actualmente es un TODO)
    auto matches = strategy.GeneratePlayoffMatches(groups, tournamentId);

    // 3. Assert: 
    // 8 partidos de Octavos (16 equipos)
    // 4 partidos de Cuartos (vacíos)
    // 2 partidos de Semis (vacíos)
    // 1 partido de Final (vacío)
    // Total = 8 + 4 + 2 + 1 = 15 partidos
    ASSERT_EQ(matches.size(), 15);
    
    // Verificar que los primeros 8 sean Octavos y tengan equipos
    for (int i = 0; i < 8; ++i) {
        ASSERT_EQ(matches[i].Phase(), MatchPhase::ROUND_OF_16);
        ASSERT_TRUE(matches[i].HasBothTeams());
    }
    // Verificar que los 4 de Cuartos estén vacíos
    for (int i = 8; i < 12; ++i) {
        ASSERT_EQ(matches[i].Phase(), MatchPhase::QUARTERFINALS);
        ASSERT_FALSE(matches[i].HasBothTeams());
    }
    // Verificar Semis y Final
    ASSERT_EQ(matches[12].Phase(), MatchPhase::SEMIFINALS);
    ASSERT_EQ(matches[13].Phase(), MatchPhase::SEMIFINALS);
    ASSERT_EQ(matches[14].Phase(), MatchPhase::FINALS);
}

// --- Prueba para el Escenario 4: Avance en Playoffs ---
TEST(IMatchStrategyTest, PlayoffAdvancement_WinnerMovesToNextMatch) {
    SingleEliminationStrategy strategy;
    
    // 1. Setup: Crear un partido de Cuartos de Final (QF Match 1)
    Match qfMatch("tourn-1", MatchPhase::QUARTERFINALS, 1); // MatchNumber 1 (impar)
    qfMatch.SetTeam1("t1"); // Equipo 1
    qfMatch.SetTeam2("t5"); // Equipo 5
    
    // Simular el resultado
    qfMatch.SetScore(3, 0); // Gana Equipo 1
    
    // Crear la estructura de partidos existentes (vacía para esta prueba)
    std::vector<std::shared_ptr<domain::Match>> existingMatches;

    // 2. Action: Generar el siguiente partido (Semifinal)
    auto nextMatchOpt = strategy.GenerateNextMatch(qfMatch, existingMatches);
    
    // 3. Assert:
    ASSERT_TRUE(nextMatchOpt.has_value()); // Debe haber un siguiente partido
    auto sfMatch = nextMatchOpt.value();
    
    // Verificar que es la Semifinal 1
    ASSERT_EQ(sfMatch.Phase(), MatchPhase::SEMIFINALS);
    ASSERT_EQ(sfMatch.MatchNumber(), 1); // (1+1)/2 = 1
    
    // Verificar que el ganador (t1) avanzó a la ranura Team1 (porque el MatchNumber 1 es impar)
    ASSERT_TRUE(sfMatch.Team1Id().has_value());
    ASSERT_EQ(sfMatch.Team1Id().value(), "t1"); // Ganador
    ASSERT_FALSE(sfMatch.Team2Id().has_value()); // El otro slot está vacío
}