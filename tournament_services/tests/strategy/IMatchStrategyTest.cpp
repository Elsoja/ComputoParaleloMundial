#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "domain/IMatchStrategy.hpp"
#include "domain/Group.hpp"
#include "domain/Team.hpp"
#include "domain/Match.hpp"
#include <map>
#include <memory> // Incluir para std::shared_ptr y std::make_shared

using namespace domain;

// ============================================================================
// TESTS DE FASE DE GRUPOS
// ============================================================================

TEST(IMatchStrategyTest, GroupStageGeneration_SingleGroup) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "tourn-1";
    
    // Usar el constructor correcto de Group(name, id)
    Group groupA("Grupo A", "group-a");
    groupA.TournamentId() = tournamentId;
    groupA.SetMaxTeams(4);
    
    // Usar .Teams().push_back()
    groupA.Teams().push_back(Team{"t1", "Equipo 1"});
    groupA.Teams().push_back(Team{"t2", "Equipo 2"});
    groupA.Teams().push_back(Team{"t3", "Equipo 3"});
    groupA.Teams().push_back(Team{"t4", "Equipo 4"});
    
    std::vector<Group> groups = {groupA};

    // Action: Generar los partidos
    auto matches = strategy.GenerateGroupStageMatches(groups, tournamentId);

    // Assert: 4 equipos = 6 partidos (n*(n-1)/2 = 4*3/2 = 6)
    ASSERT_EQ(matches.size(), 6);
    
    for (const auto& match : matches) {
        ASSERT_EQ(match.TournamentId(), tournamentId);
        ASSERT_EQ(match.Phase(), MatchPhase::GROUP_STAGE);
        // ✅ CAMBIO: Quitado .value() de un std::string
        ASSERT_EQ(match.GroupId(), "group-a"); 
    }
    
    // Verificar emparejamientos round-robin
    EXPECT_EQ(matches[0].Team1Id().value(), "t1");
    EXPECT_EQ(matches[0].Team2Id().value(), "t2");
    
    EXPECT_EQ(matches[1].Team1Id().value(), "t1");
    EXPECT_EQ(matches[1].Team2Id().value(), "t3");
    
    EXPECT_EQ(matches[2].Team1Id().value(), "t1");
    EXPECT_EQ(matches[2].Team2Id().value(), "t4");
    
    EXPECT_EQ(matches[3].Team1Id().value(), "t2");
    EXPECT_EQ(matches[3].Team2Id().value(), "t3");
    
    EXPECT_EQ(matches[4].Team1Id().value(), "t2");
    EXPECT_EQ(matches[4].Team2Id().value(), "t4");
    
    EXPECT_EQ(matches[5].Team1Id().value(), "t3");
    EXPECT_EQ(matches[5].Team2Id().value(), "t4");
}

TEST(IMatchStrategyTest, GroupStageGeneration_MultipleGroups) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "tourn-mundial";
    
    std::vector<Group> groups;
    char groupNames[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    
    for (int i = 0; i < 8; ++i) {
        std::string groupId = std::string("group-") + groupNames[i];
        std::string groupName = std::string("Grupo ") + groupNames[i];
        
        Group g(groupName, groupId);
        g.TournamentId() = tournamentId;
        g.SetMaxTeams(4);
        
        for (int j = 1; j <= 4; ++j) {
            std::string teamId = std::string(1, groupNames[i]) + std::to_string(j);
            std::string teamName = "Equipo " + teamId;
            g.Teams().push_back(Team(teamId, teamName));
        }
        groups.push_back(g);
    }

    // Action: Generar partidos de grupos
    auto matches = strategy.GenerateGroupStageMatches(groups, tournamentId);

    // Assert: 8 grupos × 6 partidos = 48 partidos
    ASSERT_EQ(matches.size(), 48);
    
    std::map<std::string, int> matchesPerGroup;
    for (const auto& match : matches) {
        // ✅ CAMBIO: Quitado .value() de un std::string
        matchesPerGroup[match.GroupId()]++;
    }
    
    ASSERT_EQ(matchesPerGroup.size(), 8);
    for (const auto& [grpId, count] : matchesPerGroup) {
        EXPECT_EQ(count, 6) << "Grupo " << grpId << " debe tener 6 partidos";
    }
}

// ============================================================================
// TESTS DE PLAYOFFS - GENERACIÓN DEL ÁRBOL
// ============================================================================

TEST(IMatchStrategyTest, PlayoffGeneration_FullWorldCupBracket) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "mundial-2026";
    
    std::vector<Group> groups;
    char groupNames[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    
    for (int i = 0; i < 8; ++i) {
        std::string groupId = std::string("group-") + groupNames[i];
        std::string groupName = std::string("Grupo ") + groupNames[i];
        
        Group g(groupName, groupId);
        g.TournamentId() = tournamentId;
        g.SetMaxTeams(4);
        
        for (int j = 1; j <= 4; ++j) {
            std::string teamId = std::string(1, groupNames[i]) + std::to_string(j);
            std::string teamName = "Equipo " + teamId;
            g.Teams().push_back(Team(teamId, teamName));
        }
        groups.push_back(g);
    }

    // Action: Generar playoffs
    auto matches = strategy.GeneratePlayoffMatches(groups, tournamentId);

    // Assert: 15 partidos (8 octavos, 4 cuartos, 2 semis, 1 final)
    ASSERT_EQ(matches.size(), 15);
    
    // Separar por fase
    std::vector<Match> octavos, cuartos, semis, final;
    for (const auto& match : matches) {
        switch (match.Phase()) {
            case MatchPhase::ROUND_OF_16:
                octavos.push_back(match);
                break;
            case MatchPhase::QUARTERFINALS:
                cuartos.push_back(match);
                break;
            case MatchPhase::SEMIFINALS:
                semis.push_back(match);
                break;
            case MatchPhase::FINALS:
                final.push_back(match);
                break;
            default:
                FAIL() << "Fase inesperada: " << static_cast<int>(match.Phase());
        }
    }
    
    // Validar cantidades
    ASSERT_EQ(octavos.size(), 8);
    ASSERT_EQ(cuartos.size(), 4);
    ASSERT_EQ(semis.size(), 2);
    ASSERT_EQ(final.size(), 1);
    
    // Validar octavos tienen equipos asignados
    for (const auto& match : octavos) {
        EXPECT_TRUE(match.HasBothTeams()) 
            << "Partido " << match.MatchNumber() << " de octavos debe tener ambos equipos";
    }
    
    // Validar rondas siguientes están vacías
    for (const auto& match : cuartos) {
        EXPECT_FALSE(match.HasBothTeams()) 
            << "Partido " << match.MatchNumber() << " de cuartos debe estar vacío";
    }
    for (const auto& match : semis) {
        EXPECT_FALSE(match.HasBothTeams());
    }
    EXPECT_FALSE(final[0].HasBothTeams());
}

TEST(IMatchStrategyTest, PlayoffGeneration_SpecificMatchups) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "mundial-test";
    
    std::vector<Group> groups;
    char groupNames[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    
    for (int i = 0; i < 8; ++i) {
        std::string groupId = std::string("group-") + groupNames[i];
        
        Group g(std::string("Grupo ") + groupNames[i], groupId);
        g.TournamentId() = tournamentId;
        g.SetMaxTeams(4);
        
        g.Teams().push_back(Team(std::string(1, groupNames[i]) + "1", "Primero " + std::string(1, groupNames[i])));
        g.Teams().push_back(Team(std::string(1, groupNames[i]) + "2", "Segundo " + std::string(1, groupNames[i])));
        g.Teams().push_back(Team("x3", "Tercero"));
        g.Teams().push_back(Team("x4", "Cuarto"));
        
        groups.push_back(g);
    }

    // Action: Generar playoffs
    auto matches = strategy.GeneratePlayoffMatches(groups, tournamentId);
    
    // Extraer solo octavos
    std::vector<Match> octavos;
    for (const auto& match : matches) {
        if (match.Phase() == MatchPhase::ROUND_OF_16) {
            octavos.push_back(match);
        }
    }
    
    // Assert: Emparejamientos específicos estilo Mundial
    ASSERT_EQ(octavos.size(), 8);
    
    // Partido 1: A1 vs H2
    EXPECT_EQ(octavos[0].Team1Id().value(), "A1");
    EXPECT_EQ(octavos[0].Team2Id().value(), "H2");
    
    // Partido 2: B1 vs G2
    EXPECT_EQ(octavos[1].Team1Id().value(), "B1");
    EXPECT_EQ(octavos[1].Team2Id().value(), "G2");
    
    // Partido 3: C1 vs F2
    EXPECT_EQ(octavos[2].Team1Id().value(), "C1");
    EXPECT_EQ(octavos[2].Team2Id().value(), "F2");
    
    // Partido 4: D1 vs E2
    EXPECT_EQ(octavos[3].Team1Id().value(), "D1");
    EXPECT_EQ(octavos[3].Team2Id().value(), "E2");
    
    // Partido 5: E1 vs D2
    EXPECT_EQ(octavos[4].Team1Id().value(), "E1");
    EXPECT_EQ(octavos[4].Team2Id().value(), "D2");
    
    // Partido 6: F1 vs C2
    EXPECT_EQ(octavos[5].Team1Id().value(), "F1");
    EXPECT_EQ(octavos[5].Team2Id().value(), "C2");
    
    // Partido 7: G1 vs B2
    EXPECT_EQ(octavos[6].Team1Id().value(), "G1");
    EXPECT_EQ(octavos[6].Team2Id().value(), "B2");
    
    // Partido 8: H1 vs A2
    EXPECT_EQ(octavos[7].Team1Id().value(), "H1");
    EXPECT_EQ(octavos[7].Team2Id().value(), "A2");
}

// ============================================================================
// TESTS DE AVANCE EN PLAYOFFS
// ============================================================================

TEST(IMatchStrategyTest, PlayoffAdvancement_OddMatchNumberGoesToTeam1) {
    SingleEliminationStrategy strategy;
    
    // Setup: Partido 1 de octavos (impar)
    Match octavoMatch("tourn-1", MatchPhase::ROUND_OF_16, 1);
    octavoMatch.SetTeam1("A1");
    octavoMatch.SetTeam2("H2");
    octavoMatch.SetScore(2, 1); // Gana A1
    
    // ✅ CAMBIO: El tipo de vector ahora es de punteros (shared_ptr)
    std::vector<std::shared_ptr<domain::Match>> existingMatches;

    // Action: Generar siguiente partido
    auto nextMatchOpt = strategy.GenerateNextMatch(octavoMatch, existingMatches);
    
    // Assert: Ganador va a Team1 del cuarto de final
    ASSERT_TRUE(nextMatchOpt.has_value());
    auto cuartoMatch = nextMatchOpt.value();
    
    EXPECT_EQ(cuartoMatch.Phase(), MatchPhase::QUARTERFINALS);
    EXPECT_EQ(cuartoMatch.MatchNumber(), 1); // (1+1)/2 = 1
    EXPECT_EQ(cuartoMatch.Team1Id().value(), "A1");
    EXPECT_FALSE(cuartoMatch.Team2Id().has_value());
}

TEST(IMatchStrategyTest, PlayoffAdvancement_EvenMatchNumberGoesToTeam2) {
    SingleEliminationStrategy strategy;
    
    // Setup: Partido 2 de octavos (par)
    Match octavoMatch("tourn-1", MatchPhase::ROUND_OF_16, 2);
    octavoMatch.SetTeam1("B1");
    octavoMatch.SetTeam2("G2");
    octavoMatch.SetScore(3, 2); // Gana B1
    
    // ✅ CAMBIO: El tipo de vector ahora es de punteros (shared_ptr)
    std::vector<std::shared_ptr<domain::Match>> existingMatches;

    // Action
    auto nextMatchOpt = strategy.GenerateNextMatch(octavoMatch, existingMatches);
    
    // Assert: Ganador va a Team2 del cuarto de final
    ASSERT_TRUE(nextMatchOpt.has_value());
    auto cuartoMatch = nextMatchOpt.value();
    
    EXPECT_EQ(cuartoMatch.Phase(), MatchPhase::QUARTERFINALS);
    EXPECT_EQ(cuartoMatch.MatchNumber(), 1); // (2+1)/2 = 1
    EXPECT_FALSE(cuartoMatch.Team1Id().has_value());
    EXPECT_EQ(cuartoMatch.Team2Id().value(), "B1");
}

TEST(IMatchStrategyTest, PlayoffAdvancement_CompleteQuarterFinal) {
    SingleEliminationStrategy strategy;
    
    // Setup: Simular que se completaron 2 octavos y ahora el cuarto está completo
    Match cuartoMatch("tourn-1", MatchPhase::QUARTERFINALS, 1);
    cuartoMatch.SetTeam1("A1"); // Ganador de octavo 1
    cuartoMatch.SetTeam2("B1"); // Ganador de octavo 2
    cuartoMatch.SetScore(1, 0); // Gana A1
    
    // ✅ CAMBIO: El tipo de vector ahora es de punteros (shared_ptr)
    std::vector<std::shared_ptr<domain::Match>> existingMatches;

    // Action
    auto nextMatchOpt = strategy.GenerateNextMatch(cuartoMatch, existingMatches);
    
    // Assert: Avanza a semifinal
    ASSERT_TRUE(nextMatchOpt.has_value());
    auto semiMatch = nextMatchOpt.value();
    
    EXPECT_EQ(semiMatch.Phase(), MatchPhase::SEMIFINALS);
    EXPECT_EQ(semiMatch.MatchNumber(), 1);
    EXPECT_EQ(semiMatch.Team1Id().value(), "A1");
}

TEST(IMatchStrategyTest, PlayoffAdvancement_NoNextMatchAfterFinals) {
    SingleEliminationStrategy strategy;
    
    // Setup: Partido de final
    Match finalMatch("tourn-1", MatchPhase::FINALS, 1);
    finalMatch.SetTeam1("A1");
    finalMatch.SetTeam2("B1");
    finalMatch.SetScore(2, 1); // Gana A1
    
    // ✅ CAMBIO: El tipo de vector ahora es de punteros (shared_ptr)
    std::vector<std::shared_ptr<domain::Match>> existingMatches;

    // Action
    auto nextMatchOpt = strategy.GenerateNextMatch(finalMatch, existingMatches);
    
    // Assert: No hay siguiente partido
    EXPECT_FALSE(nextMatchOpt.has_value());
}

// ============================================================================
// TEST DE FLUJO COMPLETO
// ============================================================================

TEST(IMatchStrategyTest, FullTournamentFlow_SimulateWorldCup) {
    SingleEliminationStrategy strategy;
    std::string tournamentId = "mundial-simulacion";
    
    std::vector<Group> groups;
    char groupNames[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'};
    
    for (int i = 0; i < 8; ++i) {
        std::string groupId = std::string("group-") + groupNames[i];
        
        Group g(std::string("Grupo ") + groupNames[i], groupId);
        g.TournamentId() = tournamentId;
        g.SetMaxTeams(4);
        
        for (int j = 1; j <= 4; ++j) {
            g.Teams().push_back(Team(std::string(1, groupNames[i]) + std::to_string(j), 
                                "Equipo " + std::string(1, groupNames[i]) + std::to_string(j)));
        }
        groups.push_back(g);
    }
    
    // 2. Generar fase de grupos
    auto groupMatches = strategy.GenerateGroupStageMatches(groups, tournamentId);
    ASSERT_EQ(groupMatches.size(), 48);
    
    // 3. Generar playoffs (incluye octavos + cuartos + semis + final vacíos)
    auto playoffMatches = strategy.GeneratePlayoffMatches(groups, tournamentId);
    ASSERT_EQ(playoffMatches.size(), 15);
    
    // 4. Contar partidos por fase
    std::map<MatchPhase, int> matchesByPhase;
    for (const auto& match : playoffMatches) {
        matchesByPhase[match.Phase()]++;
    }
    
    // 5. Validar estructura generada
    EXPECT_EQ(matchesByPhase[MatchPhase::ROUND_OF_16], 8) << "Debe haber 8 octavos";
    EXPECT_EQ(matchesByPhase[MatchPhase::QUARTERFINALS], 4) << "Debe haber 4 cuartos (vacíos)";
    EXPECT_EQ(matchesByPhase[MatchPhase::SEMIFINALS], 2) << "Debe haber 2 semis (vacías)";
    EXPECT_EQ(matchesByPhase[MatchPhase::FINALS], 1) << "Debe haber 1 final (vacía)";
    
    // 6. Validar que octavos tienen equipos y el resto no
    for (const auto& match : playoffMatches) {
        if (match.Phase() == MatchPhase::ROUND_OF_16) {
            EXPECT_TRUE(match.HasBothTeams()) << "Octavo #" << match.MatchNumber() << " debe tener equipos";
        } else {
            EXPECT_FALSE(match.HasBothTeams()) << "Partido de fase " << static_cast<int>(match.Phase()) 
                                               << " debe estar vacío";
        }
    }
}

// ============================================================================
// TESTS DE VALIDACIÓN
// ============================================================================

TEST(IMatchStrategyTest, AreGroupsFull_ReturnsFalseWhenEmpty) {
    SingleEliminationStrategy strategy;
    std::vector<Group> groups;
    
    EXPECT_FALSE(strategy.AreGroupsFull(groups));
}

TEST(IMatchStrategyTest, AreGroupsFull_ReturnsFalseWhenIncomplete) {
    SingleEliminationStrategy strategy;
    
    Group groupA("Grupo A", "group-a");
    groupA.TournamentId() = "tourn-1";
    groupA.SetMaxTeams(4);
    
    groupA.Teams().push_back(Team("t1", "Equipo 1"));
    groupA.Teams().push_back(Team("t2", "Equipo 2"));
    // Solo 2 de 4
    
    std::vector<Group> groups = {groupA};
    
    EXPECT_FALSE(strategy.AreGroupsFull(groups));
}

TEST(IMatchStrategyTest, AreGroupsFull_ReturnsTrueWhenAllFull) {
    SingleEliminationStrategy strategy;
    
    Group groupA("Grupo A", "group-a");
    groupA.TournamentId() = "tourn-1";
    groupA.SetMaxTeams(4);
    groupA.Teams().push_back(Team("t1", "Equipo 1"));
    groupA.Teams().push_back(Team("t2", "Equipo 2"));
    groupA.Teams().push_back(Team("t3", "Equipo 3"));
    groupA.Teams().push_back(Team("t4", "Equipo 4"));
    
    Group groupB("Grupo B", "group-b");
    groupB.TournamentId() = "tourn-1";
    groupB.SetMaxTeams(4);
    groupB.Teams().push_back(Team("t5", "Equipo 5"));
    groupB.Teams().push_back(Team("t6", "Equipo 6"));
    groupB.Teams().push_back(Team("t7", "Equipo 7"));
    groupB.Teams().push_back(Team("t8", "Equipo 8"));
    
    std::vector<Group> groups = {groupA, groupB};
    
    EXPECT_TRUE(strategy.AreGroupsFull(groups));
}
