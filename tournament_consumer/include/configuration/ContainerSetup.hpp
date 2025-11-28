//
// Created by tomas on 9/7/25.
//

#ifndef TOURNAMENTS_CONSUMER_CONTAINER_SETUP_HPP
#define TOURNAMENTS_CONSUMER_CONTAINER_SETUP_HPP

#include <Hypodermic/Hypodermic.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <memory>
#include <format>
#include <iostream>

#include "configuration/DatabaseConfiguration.hpp"
#include "cms/ConnectionManager.hpp"
#include "cms/QueueMessageConsumer.hpp"
#include "persistence/configuration/PostgresConnectionProvider.hpp"
#include "persistence/repository/IRepository.hpp"
#include "persistence/repository/TeamRepository.hpp"
#include "persistence/repository/TournamentRepository.hpp"
#include "persistence/repository/GroupRepository.hpp"

// 🆕 INCLUDES DE MATCHES
#include "persistence/repository/IMatchRepository.hpp"
#include "persistence/repository/MatchRepository.hpp"
#include "service/MatchService.hpp"
#include "handlers/MatchEventHandler.hpp"
#include "delegate/MatchDelegate.hpp"

namespace config {
    inline std::shared_ptr<Hypodermic::Container> containerSetup() {
        Hypodermic::ContainerBuilder builder;

        // Cargar configuración
        std::ifstream file("configuration.json");
        nlohmann::json configuration;
        file >> configuration;

        // ====================================================================
        // BASE DE DATOS
        // ====================================================================
        std::shared_ptr<PostgresConnectionProvider> postgressConnection = 
            std::make_shared<PostgresConnectionProvider>(
                configuration["databaseConfig"]["connectionString"].get<std::string>(), 
                configuration["databaseConfig"]["poolSize"].get<size_t>()
            );
        builder.registerInstance(postgressConnection).as<IDbConnectionProvider>();

        // Registrar DatabaseConfiguration para MatchRepository
        auto dbConfig = std::make_shared<configuration::DatabaseConfiguration>(
            configuration["databaseConfig"]["connectionString"].get<std::string>()
        );
        builder.registerInstance(dbConfig);

        // ====================================================================
        // ACTIVE MQ
        // ====================================================================
        builder.registerType<ConnectionManager>()
            .onActivated([configuration](Hypodermic::ComponentContext& context, 
                                        const std::shared_ptr<ConnectionManager>& instance) {
                instance->initialize(configuration["activemq"]["broker-url"].get<std::string>());
            })
            .singleInstance();

        builder.registerType<cms::QueueMessageConsumer>().singleInstance();

        // ====================================================================
        // REPOSITORIOS EXISTENTES
        // ====================================================================
        builder.registerType<TeamRepository>()
            .as<IRepository<domain::Team, std::string>>()
            .singleInstance();

        builder.registerType<TournamentRepository>()
            .as<IRepository<domain::Tournament, std::string>>()
            .singleInstance();

        // Asumiendo que tienes GroupRepository
        builder.registerType<GroupRepository>()
            .as<IRepository<domain::Group, std::string>>()
            .singleInstance();

        // ====================================================================
        //
        // ====================================================================
        builder.registerType<repository::MatchRepository>()
            .as<repository::IMatchRepository>()
            .singleInstance();

        // ====================================================================
        //
        // ====================================================================
        builder.registerType<service::MatchService>()
            .singleInstance();

        // ====================================================================
        //
        // ====================================================================
        builder.registerType<handlers::MatchEventHandler>()
            .singleInstance();

        // ====================================================================
        //
        // ====================================================================
        builder.registerType<delegate::MatchDelegate>()
            .singleInstance();

        std::cout << "✅ Container configured successfully" << std::endl;
        
        return builder.build();
    }
}

#endif //TOURNAMENTS_CONSUMER_CONTAINER_SETUP_HPP