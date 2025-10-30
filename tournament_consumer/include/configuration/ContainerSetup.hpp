//
// Created by tomas on 9/7/25.
//

#ifndef TOURNAMENTS_CONSUMER_CONTAINER_SETUP_HPP
#define TOURNAMENTS_CONSUMER_CONTAINER_SETUP_HPP

#include <Hypodermic/Hypodermic.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <print>
#include <stdexcept> // Necesario para std::runtime_error

#include "configuration/DatabaseConfiguration.hpp"
#include "cms/ConnectionManager.hpp"
#include "persistence/repository/IRepository.hpp"
#include "persistence/repository/TeamRepository.hpp"
#include "persistence/configuration/PostgresConnectionProvider.hpp"
#include "persistence/repository/TournamentRepository.hpp"
#include "cms/QueueMessageConsumer.hpp"

namespace config {
    inline std::shared_ptr<Hypodermic::Container> containerSetup() {
        Hypodermic::ContainerBuilder builder;

        // --- INICIO: CAMBIO CRÍTICO (RUTA ABSOLUTA) ---
        // Dejamos de usar rutas relativas. Usamos la ruta completa y exacta
        // al archivo de configuración fuente.
        // NOTA: Se usan dobles diagonales invertidas (\\) en C++.
        std::string config_path = "C:\\Users\\mario\\OneDrive\\ComputoParaleloMundial\\tournament_consumer\\configuration.json";
        std::ifstream file(config_path);

        // Añadimos una verificación para dar un error claro si no se abre
        if (!file.is_open()) {
            // Esto será capturado por el bloque catch en main.cpp
            throw std::runtime_error("ERROR FATAL: No se pudo abrir el archivo de configuracion en la ruta: " + config_path);
        }
        // --- FIN: CAMBIO CRÍTICO ---

        nlohmann::json configuration;
        file >> configuration; // <--- El error de JSON vacío no debería ocurrir ahora.

        std::shared_ptr<PostgresConnectionProvider> postgressConnection = std::make_shared<PostgresConnectionProvider>(configuration["databaseConfig"]["connectionString"].get<std::string>(), configuration["databaseConfig"]["poolSize"].get<size_t>());
        builder.registerInstance(postgressConnection).as<IDbConnectionProvider>();

        builder.registerType<ConnectionManager>()
            .onActivated([configuration](Hypodermic::ComponentContext& context, const std::shared_ptr<ConnectionManager>& instance) {
                instance->initialize(configuration["activemq"]["broker-url"].get<std::string>());
            })
            .singleInstance();

        builder.registerType<QueueMessageConsumer>();
            // .onActivated([](Hypodermic::ComponentContext& , const std::shared_ptr<QueueMessageConsumer>& instance) {
            //     instance->QueueName() = "tournament.created";
            //     instance->start();
            // }).singleInstance();

        // builder.registerType<QueueMessageProducer>().named("tournamentAddTeamQueue");
        // builder.registerType<QueueResolver>().as<IResolver<IQueueMessageProducer> >().named("queueResolver").
        //         singleInstance();

        builder.registerType<TeamRepository>().as<IRepository<domain::Team, std::string>>().singleInstance();

        builder.registerType<TournamentRepository>().as<IRepository<domain::Tournament, std::string>>().singleInstance();

        return builder.build();
    }
}
#endif //TOURNAMENTS_CONSUMER_CONTAINER_SETUP_HPP