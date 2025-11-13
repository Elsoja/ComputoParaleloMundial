#include "handlers/MatchEventHandler.hpp"
#include <iostream>
#include <string> 
#include <map>   
#include <vector> 
#include <memory> 

namespace handlers {

MatchEventHandler::MatchEventHandler(std::shared_ptr<repository::IMatchRepository> matchRepo)
    : matchRepository(std::move(matchRepo)) {}

void MatchEventHandler::Subscribe() {
    auto eventBus = events::EventBus::Instance();
    
    eventBus->Subscribe("ScoreRegistered", [this](const events::Event& event) {
        // Usamos dynamic_cast para asegurarnos de que el tipo de evento es correcto
        if (const auto* scoreEvent = dynamic_cast<const events::ScoreRegisteredEvent*>(&event)) {
            this->OnScoreRegistered(*scoreEvent);
        }
    });
}

void MatchEventHandler::OnScoreRegistered(const events::ScoreRegisteredEvent& event) {
    auto phase = event.Phase();
    std::cout << "[MatchEventHandler] Score registered for match " << event.MatchId() 
              << " in phase " << phase << std::endl;
    
    if (phase == "GROUP_STAGE") {
        HandleGroupStageScoreRegistered(event);
    } else {
        HandlePlayoffScoreRegistered(event);
    }
}

void MatchEventHandler::HandleGroupStageScoreRegistered(const events::ScoreRegisteredEvent& event) {
    std::string tournamentId = event.TournamentId();
    std::cout << "[MatchEventHandler] Checking if group stage is complete for tournament " 
              << tournamentId << std::endl;
    
    if (IsGroupStageComplete(tournamentId)) {
        std::cout << "[MatchEventHandler] Group stage complete! Generating playoffs..." << std::endl;
        GeneratePlayoffsFromGroupStage(tournamentId);
    }
}

void MatchEventHandler::HandlePlayoffScoreRegistered(const events::ScoreRegisteredEvent& event) {
    std::string matchId = event.MatchId();
    std::string winnerId = event.WinnerId();
    
    if (winnerId.empty()) { // Comprobar string vacío
        std::cout << "[MatchEventHandler] No winner defined for match " << matchId << std::endl;
        return;
    }

    auto completedMatch = matchRepository->ReadById(matchId);
    if (!completedMatch) {
        std::cout << "[MatchEventHandler] Match " << matchId << " not found" << std::endl;
        return;
    }

    if (event.Phase() == "FINALS") {
        std::cout << "[MatchEventHandler] Finals completed! Tournament finished." << std::endl;
        return;
    }

    auto strategy = domain::MatchStrategyFactory::CreateStrategy();
    
    if (completedMatch->NextMatchId().has_value()) {
        std::cout << "[MatchEventHandler] Updating next match with winner" << std::endl;
        UpdateNextMatchWithWinner(*completedMatch, winnerId, strategy.get());
    } else {
        std::cout << "[MatchEventHandler] Creating next match" << std::endl;
        CreateAndAssignNextMatch(*completedMatch, winnerId, strategy.get());
    }
}

bool MatchEventHandler::IsGroupStageComplete(const std::string& tournamentId) {
    return matchRepository->IsGroupStageComplete(tournamentId);
}

void MatchEventHandler::GeneratePlayoffsFromGroupStage(const std::string& tournamentId) {
    try {
        auto groupIds = GetGroupIdsForTournament(tournamentId);
        auto qualifiedTeams = ApplyStandingsRules(tournamentId, groupIds);

        if (qualifiedTeams.empty()) {
            std::cout << "[MatchEventHandler] No qualified teams found" << std::endl;
            return;
        }

        std::cout << "[MatchEventHandler] " << qualifiedTeams.size() 
                  << " teams qualified for playoffs" << std::endl;

        auto strategy = domain::MatchStrategyFactory::CreateStrategy();

        auto playoffMatches = GeneratePlayoffMatchesFromQualified(
            qualifiedTeams, 
            tournamentId, 
            strategy.get()
        );

        for (auto& match : playoffMatches) {
            matchRepository->Save(match);
        }
        std::cout << "[MatchEventHandler] Generated " << playoffMatches.size() 
                  << " playoff matches" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[MatchEventHandler] Error generating playoffs: " << e.what() << std::endl;
    }
}

std::vector<std::string> MatchEventHandler::ApplyStandingsRules(
    const std::string& tournamentId, 
    const std::vector<std::string>& groupIds
) {
    std::vector<std::string> qualifiedTeams;
    
    for (const std::string& groupId : groupIds) {
        auto standings = CalculateGroupStandings(groupId);
        
        for (size_t i = 0; i < standings.size() && i < 2; i++) {
            std::cout << "  " << (i+1) << ". Team " << standings[i].teamId 
                      << " - Wins: " << standings[i].wins 
                      << " PF: " << standings[i].pointsFor
                      << " PA: " << standings[i].pointsAgainst << std::endl;
        }
        
        if (standings.size() >= 2) {
            qualifiedTeams.push_back(standings[0].teamId); // Primero
            qualifiedTeams.push_back(standings[1].teamId); // Segundo
        } else if (standings.size() == 1) {
            qualifiedTeams.push_back(standings[0].teamId);
        }
    }
    return qualifiedTeams;
}

std::vector<MatchEventHandler::TeamStanding> MatchEventHandler::CalculateGroupStandings(const std::string& groupId) {
    auto matches = matchRepository->FindByGroupId(groupId);
    std::map<std::string, TeamStanding> standings;

    for (const auto& matchPtr : matches) {
        if (!matchPtr->IsComplete() || !matchPtr->Team1Id().has_value() || !matchPtr->Team2Id().has_value()) {
            continue;
        }

        std::string team1Id = matchPtr->Team1Id().value();
        std::string team2Id = matchPtr->Team2Id().value();
        int score1 = matchPtr->Team1Score().value_or(0);
        int score2 = matchPtr->Team2Score().value_or(0);

        if (standings.find(team1Id) == standings.end()) {
            standings[team1Id] = TeamStanding{team1Id};
        }
        if (standings.find(team2Id) == standings.end()) {
            standings[team2Id] = TeamStanding{team2Id};
        }

        standings[team1Id].matchesPlayed++;
        standings[team2Id].matchesPlayed++;
        standings[team1Id].pointsFor += score1;
        standings[team1Id].pointsAgainst += score2;
        standings[team2Id].pointsFor += score2;
        standings[team2Id].pointsAgainst += score1;

        if (score1 > score2) {
            standings[team1Id].wins++;
            standings[team2Id].losses++;
        } else if (score2 > score1) {
            standings[team2Id].wins++;
            standings[team1Id].losses++;
        } else {
            standings[team1Id].draws++;
            standings[team2Id].draws++;
        }
    }

    std::vector<TeamStanding> result;
    for (const auto& [teamId, standing] : standings) {
        result.push_back(standing);
    }
    std::sort(result.begin(), result.end());
    return result;
}


std::vector<domain::Match> MatchEventHandler::GeneratePlayoffMatchesFromQualified(
    const std::vector<std::string>& qualifiedTeams,
    const std::string& tournamentId,
    domain::IMatchStrategy* strategy
) {
    std::vector<domain::Match> matches;
    if (qualifiedTeams.size() != 16) {
        throw std::runtime_error("Expected 16 qualified teams for round of 16");
    }

    std::vector<std::string> firstPlaces;
    std::vector<std::string> secondPlaces;
    
    for (size_t i = 0; i < qualifiedTeams.size(); i += 2) {
        firstPlaces.push_back(qualifiedTeams[i]);
        if (i + 1 < qualifiedTeams.size()) {
            secondPlaces.push_back(qualifiedTeams[i + 1]);
        }
    }

    int matchNumber = 1;
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[0], secondPlaces[7])); // A1 vs H2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[1], secondPlaces[6])); // B1 vs G2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[2], secondPlaces[5])); // C1 vs F2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[3], secondPlaces[4])); // D1 vs E2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[4], secondPlaces[3])); // E1 vs D2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[5], secondPlaces[2])); // F1 vs C2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[6], secondPlaces[1])); // G1 vs B2
    matches.push_back(CreateMatchWithTeams(tournamentId, domain::MatchPhase::ROUND_OF_16, 
                                           matchNumber++, firstPlaces[7], secondPlaces[0])); // H1 vs A2

    GenerateEmptyPlayoffStructure(matches, tournamentId, domain::MatchPhase::ROUND_OF_16, 16);
    return matches;
}

domain::Match MatchEventHandler::CreateMatchWithTeams(
    const std::string& tournamentId, 
    domain::MatchPhase phase, 
    int matchNumber,
    const std::string& team1Id, 
    const std::string& team2Id
) {
    domain::Match match(tournamentId, phase, matchNumber);
    match.SetTeam1(team1Id);
    match.SetTeam2(team2Id);
    return match;
}

void MatchEventHandler::UpdateNextMatchWithWinner(
    const domain::Match& completedMatch,
    const std::string& winnerId, 
    domain::IMatchStrategy* strategy
) {
    auto nextMatch = matchRepository->ReadById(completedMatch.NextMatchId().value());
    if (!nextMatch) {
        std::cout << "[MatchEventHandler] Next match not found" << std::endl;
        return;
    }
    bool isTeam1 = (completedMatch.MatchNumber() % 2) == 1;
    strategy->UpdateMatchWithWinner(*nextMatch, winnerId, isTeam1);
    matchRepository->Update(*nextMatch);
    std::cout << "[MatchEventHandler] Updated match " << nextMatch->Id() 
              << " with winner " << winnerId << std::endl;
}

void MatchEventHandler::CreateAndAssignNextMatch(
    const domain::Match& completedMatch,
    const std::string& winnerId, 
    domain::IMatchStrategy* strategy
) {
    auto existingMatches = matchRepository->FindByTournamentId(completedMatch.TournamentId());
    auto nextMatchOpt = strategy->GenerateNextMatch(completedMatch, existingMatches);
    
    if (nextMatchOpt.has_value()) {
        auto savedMatch = matchRepository->Save(nextMatchOpt.value());
        
        auto updatedMatch = completedMatch; // Copia para modificar
        updatedMatch.SetNextMatchId(savedMatch.Id());
        matchRepository->Update(updatedMatch);

        std::cout << "[MatchEventHandler] Created next match " << savedMatch.Id() 
                  << " for completed match " << completedMatch.Id() << std::endl;
    }
}

void MatchEventHandler::GenerateEmptyPlayoffStructure(
    std::vector<domain::Match>& matches,
    const std::string& tournamentId,
    domain::MatchPhase initialPhase,
    int teamCount
) {
    domain::MatchPhase currentPhase = initialPhase;
    int currentMatchCount = teamCount / 2;

    while (currentPhase != domain::MatchPhase::FINALS) {
        currentPhase = GetNextPhase(currentPhase);
        currentMatchCount = currentMatchCount / 2;
        if (currentMatchCount < 1) break;
        for (int i = 1; i <= currentMatchCount; i++) {
            domain::Match match(tournamentId, currentPhase, i);
            matches.push_back(match);
        }
        if (currentMatchCount == 1) break;
    }
}

void MatchEventHandler::LinkPlayoffMatches(std::vector<domain::Match>& matches) {
    // Agrupar por fase
    std::map<domain::MatchPhase, std::vector<size_t>> matchIndexByPhase;
    
    for (size_t i = 0; i < matches.size(); i++) {
        matchIndexByPhase[matches[i].Phase()].push_back(i);
    }

    // Vincular cada fase con la siguiente
    for (auto& [phase, indices] : matchIndexByPhase) {
        if (phase == domain::MatchPhase::FINALS) continue;

        domain::MatchPhase nextPhase = GetNextPhase(phase);
        if (matchIndexByPhase.find(nextPhase) == matchIndexByPhase.end()) continue;

        auto& nextPhaseIndices = matchIndexByPhase[nextPhase];
        
        for (size_t i = 0; i < indices.size(); i++) {
            size_t nextMatchIndex = i / 2;
            if (nextMatchIndex < nextPhaseIndices.size()) {
                // (La lógica de vinculación real sucede en UpdateNextMatchWithWinner
                // y CreateAndAssignNextMatch)
            }
        }
    }
}

domain::MatchPhase MatchEventHandler::GetNextPhase(domain::MatchPhase currentPhase) const {
    switch (currentPhase) {
        case domain::MatchPhase::ROUND_OF_16:
            return domain::MatchPhase::QUARTERFINALS;
        case domain::MatchPhase::QUARTERFINALS:
            return domain::MatchPhase::SEMIFINALS;
        case domain::MatchPhase::SEMIFINALS:
            return domain::MatchPhase::FINALS;
        default:
            return domain::MatchPhase::FINALS;
    }
}

std::vector<std::string> MatchEventHandler::GetGroupIdsForTournament(const std::string& tournamentId) {
    auto matches = matchRepository->FindByTournamentIdAndPhase(
        tournamentId, 
        domain::MatchPhase::GROUP_STAGE
    );

    std::vector<std::string> groupIds;
    for (const auto& matchPtr : matches) {
        if (!matchPtr->GroupId().empty()) { 
            const std::string& groupId = matchPtr->GroupId();
            if (std::find(groupIds.begin(), groupIds.end(), groupId) == groupIds.end()) {
                groupIds.push_back(groupId);
            }
        }
    }
    return groupIds;
}

} // namespace handlers