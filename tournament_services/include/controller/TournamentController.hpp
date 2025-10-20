#ifndef TOURNAMENTS_TOURNAMENTCONTROLLER_HPP
#define TOURNAMENTS_TOURNAMENTCONTROLLER_HPP

#include <memory>
#include <string>
#include "crow.h"
#include "delegate/ITournamentDelegate.hpp"

class TournamentController {
    std::shared_ptr<ITournamentDelegate> tournamentDelegate;
public:
    explicit TournamentController(std::shared_ptr<ITournamentDelegate> delegate);

    // Métodos existentes
    crow::response CreateTournament(const crow::request& request) const;
    crow::response ReadAll() const;

    crow::response GetTournament(const std::string& id) const;
    crow::response UpdateTournament(const crow::request& request, const std::string& id) const;
    crow::response DeleteTournament(const std::string& id) const;
};

#endif //TOURNAMENTS_TOURNAMENTCONTROLLER_HPP