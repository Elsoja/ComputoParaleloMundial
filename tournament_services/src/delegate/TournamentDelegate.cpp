//
// Created by tomas on 8/31/25.
//
#include "delegate/TournamentDelegate.hpp"
#include "persistence/repository/IRepository.hpp"
#include "cms/QueueMessageProducer.hpp" // Asegúrate de que la ruta sea correcta
#include <string_view>
#include <memory>
#include <utility> // Para std::move

// CAMBIO: Asegúrate de que el constructor coincida con tu .hpp y el código original
TournamentDelegate::TournamentDelegate(std::shared_ptr<IRepository<domain::Tournament, std::string>> repository, std::shared_ptr<QueueMessageProducer> producer) 
    : tournamentRepository(std::move(repository)), producer(std::move(producer)) {
}

// CAMBIO: El método ahora devuelve std::expected y maneja el optional del repositorio
std::expected<std::string, ITournamentDelegate::SaveError> TournamentDelegate::CreateTournament(std::shared_ptr<domain::Tournament> tournament) {
    
    // La lógica de rellenar grupos se mantiene si la necesitas
    // for (auto[i, g] = std::tuple{0, 'A'}; i < tournament->Format().NumberOfGroups(); i++,g++) {
    //     tournament->Groups().push_back(domain::Group{std::format("Tournament {}", g)});
    // }

    // 1. Llamamos al repositorio. Devuelve un std::optional<std::string>
    auto idOptional = tournamentRepository->Create(*tournament);

    // 2. Verificamos si el optional contiene un valor
    if (idOptional) {
        // ÉXITO: El torneo se creó.
        const std::string& id = idOptional.value(); // Obtenemos el ID

        // 3. Enviamos el mensaje a la cola, como en tu código original.
        producer->SendMessage(id, "tournament.created");

        // 4. Devolvemos el resultado exitoso.
        return id;
    } else {
        // FRACASO: Hubo un conflicto (torneo duplicado).
        // Devolvemos un error.
        return std::unexpected(ITournamentDelegate::SaveError::Conflict);
    }
}

std::vector<std::shared_ptr<domain::Tournament>> TournamentDelegate::ReadAll() {
    return tournamentRepository->ReadAll();
}