project(tournament_services)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Directorio de headers para todo el proyecto
include_directories(include)

# --- Búsqueda de Dependencias ---
find_path(HYPODERMIC_INCLUDE_DIRS "Hypodermic/ActivatedRegistrationInfo.h")


# --- Creación de la Librería de Lógica ---
# Se usan rutas absolutas para mayor robustez
set(SERVICES_LOGIC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delegate/TeamDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delegate/TournamentDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/TournamentController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/TeamController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/GroupController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/controller/MatchController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/service/MatchService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/handlers/MatchEventHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/delegate/MatchDelegate.cpp
)


add_library(tournament_logic STATIC ${SERVICES_LOGIC_SOURCES})

target_link_libraries(tournament_logic PUBLIC
    nlohmann_json::nlohmann_json
    tournament_common
)



# --- Creación del Ejecutable Principal ---
add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE
    tournament_logic
    Crow::Crow
    asio::asio
    libpqxx::pqxx
    unofficial::activemq-cpp::activemq-cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${HYPODERMIC_INCLUDE_DIRS})


# Enlazar con tournament_common y otras dependencias
target_link_libraries(tournament_services 
    PRIVATE 
        tournament_common
        nlohmann_json::nlohmann_json
)

# --- Configuración de Pruebas ---
include(CTest)
enable_testing()
add_subdirectory(tests)


# --- Copia de Archivos de Configuración ---
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/configuration.json
    ${CMAKE_BINARY_DIR}/configuration.json
    COPYONLY
)