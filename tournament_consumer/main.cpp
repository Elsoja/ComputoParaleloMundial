#include <activemq/library/ActiveMQCPP.h>
#include <thread>
#include <iostream>
#include <print> 
#include <memory>
#include <chrono> 

#include "configuration/ContainerSetup.hpp"
#include "cms/QueueMessageConsumer.hpp" 
#include "handlers/MatchEventHandler.hpp" 
#include "events/Events.hpp"             
#include "persistence/repository/IMatchRepository.hpp"

int main() {
    activemq::library::ActiveMQCPP::initializeLibrary();
    {
        std::println("🚀 Starting Tournament Consumer");
        std::println("📦 Initializing container...");
        
        const auto container = config::containerSetup();
        
        std::println("✅ Container initialized");

        try {
            auto matchRepo = container->resolve<repository::IMatchRepository>();
            auto eventHandler = std::make_shared<handlers::MatchEventHandler>(matchRepo);
            eventHandler->Subscribe();
            std::println("✅ MatchEventHandler subscribed to internal EventBus");
        } catch (const std::exception& e) {
            std::println(stderr, "❌ Error subscribing MatchEventHandler: {}", e.what());
        }

        std::thread tournamentCreatedThread([&] {
            try {
                std::println("🎯 Starting 'tournament.created' listener...");
                
                // ✅ SOLUCIÓN 1: Resolver sin std::shared_ptr wrapper
                auto listener = container->resolve<cms::QueueMessageConsumer>();
                
                // ✅ SOLUCIÓN 2: O crear directamente si tienes ConnectionManager
                // auto connManager = container->resolve<cms::ConnectionManager>();
                // auto listener = std::make_shared<cms::QueueMessageConsumer>(connManager);
                
                listener->Start("tournament.created");
            } catch (const std::exception& e) {
                std::println(stderr, "❌ Error in 'tournament.created' thread: {}", e.what());
            }
        });

        std::println("✅ All listeners started");
        std::println("📡 Listening to:");
        std::println("   - 'tournament.created' (External via ActiveMQ)");
        std::println("   - 'ScoreRegistered' (Internal via EventBus)");
        std::println("\n⏸️  Press ENTER to stop...\n");

        std::cin.get();

        std::println("\n🛑 Stopping consumers...");
        tournamentCreatedThread.join();
        std::println("✅ Consumers stopped gracefully");
    }
    activemq::library::ActiveMQCPP::shutdownLibrary();
    std::println("👋 Bye!");
    return 0;
}