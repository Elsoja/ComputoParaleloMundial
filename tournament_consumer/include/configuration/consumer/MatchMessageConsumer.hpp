#ifndef MATCH_MESSAGE_HANDLER_HPP
#define MATCH_MESSAGE_HANDLER_HPP

#include "delegate/MatchDelegate.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <string>

namespace consumer {

class MatchMessageHandler {
private:
    std::shared_ptr<delegate::MatchDelegate> matchDelegate;

    void HandleRegisterScore(const nlohmann::json& message);
    void HandleGetMatchesByTournament(const nlohmann::json& message);
    void HandleGetMatchesByPhase(const nlohmann::json& message);
    void HandleGetMatchesByGroup(const nlohmann::json& message);

public:
    explicit MatchMessageHandler(std::shared_ptr<delegate::MatchDelegate> delegate);

    void ProcessMessage(const std::string& messageText);
};

} // namespace consumer

#endif // MATCH_MESSAGE_HANDLER_HPP