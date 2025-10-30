//
// Created by tomas on 9/6/25.
//
#include <activemq/library/ActiveMQCPP.h>
#include <thread>
#include <print> // Necesario para std::println

// Incluir la cabecera de la excepción de ActiveMQ
#include <decaf/lang/Exception.h>

#include "configuration/ContainerSetup.hpp"
#include "cms/ConnectionManager.hpp" // Necesario para resolver el tipo

int main() {
    activemq::library::ActiveMQCPP::initializeLibrary();
    try {
        std::println("before container");
        const auto container = config::containerSetup();
        std::println("after container");

        // --- INICIO: DEBUG DE CONEXIÓN ---
        std::println("Attempting ConnectionManager initialization (ActiveMQ connection attempt)...");
        // Al resolver el ConnectionManager, se activa su inicialización gracias a .onActivated()
        auto conn_manager = container->resolve<ConnectionManager>();
        std::println("ConnectionManager successfully resolved. The initialization likely passed.");
        // --- FIN: DEBUG DE CONEXIÓN ---

        // El thread principal ahora pasa a la lógica del consumidor
        std::thread tournamentCreatedThread([&] {
            auto listener = container->resolve<QueueMessageConsumer>();
            listener->Start("tournament.created");
        });

        tournamentCreatedThread.join();
        // while (true) {
        //     std::this_thread::sleep_for(std::chrono::seconds(5));
        // }
    } catch (const decaf::lang::Exception& e) {
        // Capturamos cualquier excepción lanzada por ActiveMQ o sus dependencias
        std::println("FATAL ACTIVE MQ ERROR: {}", e.getMessage());
        return 1; // Salir con código de error
    } catch (const std::exception& e) {
        // Capturamos otras excepciones estándar de C++
        std::println("FATAL RUNTIME ERROR: {}", e.what());
        return 1;
    } catch (...) {
        // Capturamos cualquier otra cosa
        std::println("FATAL UNKNOWN ERROR during container or connection setup.");
        return 1;
    }

    activemq::library::ActiveMQCPP::shutdownLibrary();
    return 0;
}