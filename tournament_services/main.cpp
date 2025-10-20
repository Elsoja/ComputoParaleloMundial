#include <activemq/library/ActiveMQCPP.h>
#include "configuration/ContainerSetup.hpp"
#include "configuration/RouteDefinition.hpp" // <-- Este include soluciona el error
#include "configuration/RunConfiguration.hpp"  // <-- Ruta corregida

int main() {
    activemq::library::ActiveMQCPP::initializeLibrary();
    const auto container = config::containerSetup();
    crow::SimpleApp app;

    // Bind all annotated routes
    for (auto& def : routeRegistry()) {
        def.binder(app, container);
    }

    auto appConfig = container->resolve<config::RunConfiguration>();

    app.port(appConfig->port)
       .concurrency(appConfig->concurrency)
       .run();
       
    activemq::library::ActiveMQCPP::shutdownLibrary();
}