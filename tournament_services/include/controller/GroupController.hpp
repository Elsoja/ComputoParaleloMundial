#ifndef TOURNAMENTS_GROUPCONTROLLER_HPP
#define TOURNAMENTS_GROUPCONTROLLER_HPP

#include <memory>
#include <string>
#include "crow.h"
#include "delegate/IGroupDelegate.hpp"

class GroupController {
    std::shared_ptr<IGroupDelegate> groupDelegate;
public:
    explicit GroupController(const std::shared_ptr<IGroupDelegate>& delegate);

    crow::response CreateGroup(const crow::request& request, const std::string& tournamentId) const;
    crow::response GetGroups(const std::string& tournamentId) const;
    crow::response GetGroup(const std::string& tournamentId, const std::string& groupId) const;
    crow::response UpdateGroup(const crow::request& request, const std::string& tournamentId, const std::string& groupId) const;
    crow::response DeleteGroup(const std::string& tournamentId, const std::string& groupId) const;
};

#endif //TOURNAMENTS_GROUPCONTROLLER_HPP