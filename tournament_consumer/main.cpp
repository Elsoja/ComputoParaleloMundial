#include <activemq/library/ActiveMQCPP.h>
#include <thread>
#include <iostream>
#include <format> 
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
        std::cout << " Starting Tournament Consumer" << std::endl;
        std::cout << " Initializing container..." << std::endl;
        
        const auto container = config::containerSetup();
        
        std::cout << " Container initialized" << std::endl;

        try {
            auto matchRepo = container->resolve<repository::IMatchRepository>();
            auto eventHandler = std::make_shared<handlers::MatchEventHandler>(matchRepo);
            eventHandler->Subscribe();
            std::cout << " MatchEventHandler subscribed to internal EventBus" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << std::format(" Error subscribing MatchEventHandler: {}", e.what()) << std::endl;
        }

        std::thread tournamentCreatedThread([&] {
            try {
                std::cout << " Starting 'tournament.created' listener..." << std::endl;
                
                auto listener = container->resolve<cms::QueueMessageConsumer>();
                
               
                listener->Start("tournament.created");
            } catch (const std::exception& e) {
                std::cerr << std::format(" Error in 'tournament.created' thread: {}", e.what()) << std::endl;
            }
        });

        std::cout << " All listeners started" << std::endl;
        std::cout << " Listening to:" << std::endl;
        std::cout << "   - 'tournament.created' (External via ActiveMQ)" << std::endl;
        std::cout << "   - 'ScoreRegistered' (Internal via EventBus)" << std::endl;
        std::cout << "\n  Press ENTER to stop...\n" << std::endl;

        std::cin.get();

        std::cout << "\n Stopping consumers..." << std::endl;
        tournamentCreatedThread.join();
        std::cout << " Consumers stopped gracefully" << std::endl;
    }
    activemq::library::ActiveMQCPP::shutdownLibrary();
    std::cout << " Bye!" << std::endl;
    return 0;
}