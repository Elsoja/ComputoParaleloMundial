#include <activemq/library/ActiveMQCPP.h>
#include "configuration/ContainerSetup.hpp"
#include "configuration/RouteDefinition.hpp" // <-- Este include soluciona el error
#include "configuration/RunConfiguration.hpp"  // <-- Ruta corregida
#include <iostream> // <-- AGREGAR ESTE INCLUDE

int main() {
    std::cout << "=== Starting tournament_services ===" << std::endl;

    try {
        std::cout << "1. Creating container..." << std::endl;
        const auto container = config::containerSetup();
        std::cout << "   Container created successfully!" << std::endl;

        std::cout << "2. Initializing ActiveMQ library..." << std::endl;
        activemq::library::ActiveMQCPP::initializeLibrary();
        std::cout << "   ActiveMQ initialized!" << std::endl;

        std::cout << "3. Creating Crow app..." << std::endl;
        crow::SimpleApp app;
        std::cout << "   Crow app created!" << std::endl;

        std::cout << "4. Binding routes..." << std::endl;
        // Bind all annotated routes
        for (auto& def : routeRegistry()) {
            def.binder(app, container);
        }
        std::cout << "   Routes bound: " << routeRegistry().size() << " routes" << std::endl;

        std::cout << "5. Resolving run configuration..." << std::endl;
        auto appConfig = container->resolve<config::RunConfiguration>();
        std::cout << "   Port: " << appConfig->port << ", Concurrency: " << appConfig->concurrency << std::endl;

        std::cout << "6. Starting Crow server..." << std::endl;
        app.port(appConfig->port)
           .concurrency(appConfig->concurrency)
           .run();

        std::cout << "7. Shutting down ActiveMQ..." << std::endl;
        activemq::library::ActiveMQCPP::shutdownLibrary();

        std::cout << "=== Server stopped gracefully ===" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "FATAL UNKNOWN ERROR" << std::endl;
        return 1;
    }

    return 0;
}