#ifndef DOMAIN_UTILITIES_HPP
#define DOMAIN_UTILITIES_HPP

#include <string_view>
#include "domain/Tournament.hpp" // Necesario para TournamentType

namespace domain {

    // Esta es una función de utilidad y se queda aquí.
    inline TournamentType fromString(std::string_view type) {
        if (type == "ROUND_ROBIN")
            return TournamentType::ROUND_ROBIN;
        if (type == "NFL")
            return TournamentType::NFL;

        return TournamentType::ROUND_ROBIN;
    }

} // namespace domain

#endif // DOMAIN_UTILITIES_HPP