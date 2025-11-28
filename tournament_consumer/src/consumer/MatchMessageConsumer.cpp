#include <iostream>
#include <format>

namespace consumer {

MatchMessageHandler::MatchMessageHandler(std::shared_ptr<delegate::MatchDelegate> delegate)
    : matchDelegate(delegate) {}

void MatchMessageHandler::ProcessMessage(const std::string& messageText) {
    try {
        auto json = nlohmann::json::parse(messageText);
        
        if (!json.contains("message_type")) {
            std::cerr << "[MatchMessageHandler] Missing message_type field" << std::endl;
            return;
        }

        std::string messageType = json["message_type"];
        
        std::cout << std::format("[MatchMessageHandler] Processing: {}", messageType) << std::endl;

        if (messageType == "RegisterScore") {
            HandleRegisterScore(json);
        } else if (messageType == "GetMatchesByTournament") {
            HandleGetMatchesByTournament(json);
        } else if (messageType == "GetMatchesByPhase") {
            HandleGetMatchesByPhase(json);
        } else if (messageType == "GetMatchesByGroup") {
            HandleGetMatchesByGroup(json);
        } else {
            std::cerr << std::format("[MatchMessageHandler] Unknown message type: {}", messageType) << std::endl;
        }

    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << std::format("[MatchMessageHandler] JSON parse error: {}", e.what()) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << std::format("[MatchMessageHandler] Error processing message: {}", e.what()) << std::endl;
    }
}

void MatchMessageHandler::HandleRegisterScore(const nlohmann::json& message) {
    try {
        // Validar campos requeridos
        if (!message.contains("match_id") || !message.contains("team1_score") || !message.contains("team2_score")) {
            throw std::invalid_argument("Missing required fields: match_id, team1_score, team2_score");
        }

        int matchId = message["match_id"];
        int team1Score = message["team1_score"];
        int team2Score = message["team2_score"];

        std::cout << std::format("[MatchMessageHandler] Registering score for match {}: {}-{}", 
                    matchId, team1Score, team2Score) << std::endl;

        // 🔥 LLAMAR AL DELEGATE
        matchDelegate->RegisterScore(matchId, team1Score, team2Score);

        std::cout << std::format("[MatchMessageHandler] ✅ Score registered successfully for match {}", matchId) << std::endl;

        // TODO: Publicar evento de confirmación a ActiveMQ si es necesario
        // publisher->Publish("tournament.matches.score-registered", {...});

    } catch (const std::invalid_argument& e) {
        std::cerr << std::format("[MatchMessageHandler] ❌ Validation error: {}", e.what()) << std::endl;
        // TODO: Publicar mensaje de error
    } catch (const std::exception& e) {
        std::cerr << std::format("[MatchMessageHandler] ❌ Error registering score: {}", e.what()) << std::endl;
        // TODO: Publicar mensaje de error
    }
}

void MatchMessageHandler::HandleGetMatchesByTournament(const nlohmann::json& message) {
    try {
        if (!message.contains("tournament_id")) {
            throw std::invalid_argument("Missing required field: tournament_id");
        }

        int tournamentId = message["tournament_id"];

        std::cout << std::format("[MatchMessageHandler] Getting matches for tournament {}", tournamentId) << std::endl;

        auto matches = matchDelegate->GetMatchesByTournament(tournamentId);

        std::cout << std::format("[MatchMessageHandler] ✅ Found {} matches", matches.size()) << std::endl;

        // TODO: Serializar matches a JSON y publicar respuesta
        // nlohmann::json response = {
        //     {"message_type", "MatchesList"},
        //     {"tournament_id", tournamentId},
        //     {"matches", ...}
        // };
        // publisher->Publish("tournament.matches.list", response.dump());

    } catch (const std::exception& e) {
        std::cerr << std::format("[MatchMessageHandler] ❌ Error getting matches: {}", e.what()) << std::endl;
    }
}

void MatchMessageHandler::HandleGetMatchesByPhase(const nlohmann::json& message) {
    try {
        if (!message.contains("tournament_id") || !message.contains("phase")) {
            throw std::invalid_argument("Missing required fields: tournament_id, phase");
        }

        int tournamentId = message["tournament_id"];
        std::string phase = message["phase"];

        std::cout << std::format("[MatchMessageHandler] Getting matches for tournament {} phase {}", 
                    tournamentId, phase) << std::endl;

        auto matches = matchDelegate->GetMatchesByPhase(tournamentId, phase);

        std::cout << std::format("[MatchMessageHandler] ✅ Found {} matches in phase {}", matches.size(), phase) << std::endl;

        // TODO: Publicar respuesta

    } catch (const std::exception& e) {
        std::cerr << std::format("[MatchMessageHandler] ❌ Error getting matches by phase: {}", e.what()) << std::endl;
    }
}

void MatchMessageHandler::HandleGetMatchesByGroup(const nlohmann::json& message) {
    try {
        if (!message.contains("group_id")) {
            throw std::invalid_argument("Missing required field: group_id");
        }

        int groupId = message["group_id"];

        std::cout << std::format("[MatchMessageHandler] Getting matches for group {}", groupId) << std::endl;

        auto matches = matchDelegate->GetMatchesByGroup(groupId);

        std::cout << std::format("[MatchMessageHandler] ✅ Found {} matches in group", matches.size()) << std::endl;

        // TODO: Publicar respuesta

    } catch (const std::exception& e) {
        std::cerr << std::format("[MatchMessageHandler] ❌ Error getting matches by group: {}", e.what()) << std::endl;
    }
}

} // namespace consumer