#include <activemq/library/ActiveMQCPP.h>
#include <thread>
#include <iostream>
#include <print>
#include <memory>
#include <chrono> // Para std::this_thread

#include "configuration/ContainerSetup.hpp"
#include "cms/QueueMessageConsumer.hpp"
#include "handlers/MatchEventHandler.hpp" // Lo único que necesitamos para lógica de Match
#include "events/Events.hpp"              // Para el EventBus
#include "persistence/repository/IMatchRepository.hpp" // Para crear el Handler

int main() {
    activemq::library::ActiveMQCPP::initializeLibrary();
    {
        std::println("🚀 Starting Tournament Consumer");
        std::println("📦 Initializing container...");
        
        const auto container = config::containerSetup();
        
        std::println("✅ Container initialized");

        // --- 1. Suscribir Manejadores de Eventos Internos ---
        // Esto escucha los eventos publicados por el EventBus
        // (ej. cuando MatchService en la API publica ScoreRegisteredEvent)
        try {
            auto matchRepo = container->resolve<repository::IMatchRepository>();
            auto eventHandler = std::make_shared<handlers::MatchEventHandler>(matchRepo);
            eventHandler->Subscribe();
            std::println("✅ MatchEventHandler subscribed to internal EventBus");
        } catch (const std::exception& e) {
            std::println(stderr, "❌ Error subscribing MatchEventHandler: {}", e.what());
        }

        // --- 2. Iniciar Listeners de Mensajes Externos (ActiveMQ) ---
        // Esto escucha mensajes que vienen de FUERA (ej. "tournament.created")
        std::thread tournamentCreatedThread([&] {
            try {
                std::println("🎯 Starting 'tournament.created' listener...");
                auto listener = container->resolve<std::shared_ptr<cms::QueueMessageConsumer>>();
                
                // TODO: El listener->Start() necesita ser adaptado para
                // tomar un callback que *publique* un evento en el EventBus.
                // (Por ahora, lo dejamos como estaba en tu original)
                listener->Start("tournament.created");
            } catch (const std::exception& e) {
                std::println(stderr, "❌ Error in 'tournament.created' thread: {}", e.what());
            }
        });

        // ✅ CAMBIO: El "NUEVO THREAD: Match Messages" fue eliminado.
        // Ya no es necesario, porque el MatchEventHandler (Paso 1)
        // ya está manejando todos los eventos de partidos internamente.

        std::println("✅ All listeners started");
        std::println("📡 Listening to:");
        std.println("   - 'tournament.created' (External via ActiveMQ)");
        std::println("   - 'ScoreRegistered' (Internal via EventBus)");
        std::println("\n⏸️  Press ENTER to stop...\n");

        // Esperar entrada del usuario
        std::cin.get();

        std::println("\n🛑 Stopping consumers...");

        // TODO: Necesitarás una forma de detener el listener->Start()
        // para que el hilo pueda hacer join() de forma limpia.
        // Por ahora, asumimos que se detiene al cerrar.
        tournamentCreatedThread.join();

        std::println("✅ Consumers stopped gracefully");
    }
    activemq::library::ActiveMQCPP::shutdownLibrary();
    std::println("👋 Bye!");
    return 0;
}