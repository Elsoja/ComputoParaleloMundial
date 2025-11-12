#include <activemq/library/ActiveMQCPP.h>
#include "configuration/ContainerSetup.hpp"
#include "configuration/RouteDefinition.hpp" // Para routeRegistry()
#include "configuration/RunConfiguration.hpp"
#include <crow.h> // Para crow::SimpleApp

int main() {
    // Inicializar ActiveMQ
    activemq::library::ActiveMQCPP::initializeLibrary();
    
    // Crear el contenedor de dependencias
    const auto container = config::containerSetup();
    
    // Crear la aplicación web
    crow::SimpleApp app;

    // --- Registrar todas las rutas ---
    // El 'routeRegistry' encuentra automáticamente TODAS las rutas
    // (de TeamController, GroupController, MatchController, etc.)
    // que usaron la macro REGISTER_ROUTE.
    for (auto& def : routeRegistry()) {
        def.binder(app, container);
    }

    // Resolver la configuración de ejecución desde el contenedor
    auto appConfig = container->resolve<config::RunConfiguration>();

    // Iniciar el servidor
    app.port(appConfig->port)
       .concurrency(appConfig->concurrency)
       .run();
       
    // Apagar ActiveMQ al salir
    activemq::library::ActiveMQCPP::shutdownLibrary();
}